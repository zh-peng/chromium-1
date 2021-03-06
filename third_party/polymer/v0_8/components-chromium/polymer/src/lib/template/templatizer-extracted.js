

  Polymer.Templatizer = {

    templatize: function(template) {
      this._templatized = template;
      // TODO(sjmiles): supply _alternate_ content reference missing from root
      // templates (not nested). `_content` exists to provide content sharing
      // for nested templates.
      if (!template._content) {
        template._content = template.content;
      }
      // fast path if template's anonymous class has been memoized
      if (template._content._ctor) {
        this.ctor = template._content._ctor;
        //console.log('Templatizer.templatize: using memoized archetype');
        // forward parent properties to archetype
        this._prepParentProperties(this.ctor.prototype);
        return;
      }
      // `archetype` is the prototype of the anonymous
      // class created by the templatizer
      var archetype = Object.create(Polymer.Base);
      // normally Annotations.parseAnnotations(template) but
      // archetypes do special caching
      this.customPrepAnnotations(archetype, template);

      // setup accessors
      archetype._prepEffects();
      archetype._prepBehaviors();
      archetype._prepBindings();

      // forward parent properties to archetype
      this._prepParentProperties(archetype);

      // boilerplate code
      archetype._notifyPath = this._notifyPathImpl;
      archetype._scopeElementClass = this._scopeElementClassImpl;
      // boilerplate code
      var _constructor = this._constructorImpl;
      var ctor = function TemplateInstance(model, host) {
        _constructor.call(this, model, host);
      };
      // standard references
      ctor.prototype = archetype;
      archetype.constructor = ctor;
      // TODO(sjmiles): constructor cache?
      template._content._ctor = ctor;
      // TODO(sjmiles): choose less general name
      this.ctor = ctor;
    },

    _getRootDataHost: function() {
      return (this.dataHost && this.dataHost._rootDataHost) || this.dataHost;
    },

    _getAllStampedChildren: function(children) {
      children = children || [];
      if (this._getStampedChildren) {
        var c$ = this._getStampedChildren();
        for (var i=0, c; c = c$[i]; i++) {
          children.push(c);
          if (c._getAllStampedChildren) {
            c._getAllStampedChildren(children);
          }
        }
      }
      return children;
    },

    customPrepAnnotations: function(archetype, template) {
      if (template) {
        archetype._template = template;
        var c = template._content;
        if (c) {
          var rootDataHost = archetype._rootDataHost;
          if (rootDataHost) {
            Polymer.Annotations.prepElement =
              rootDataHost._prepElement.bind(rootDataHost);
          }
          archetype._notes = c._notes ||
            Polymer.Annotations.parseAnnotations(template);
          c._notes = archetype._notes;
          Polymer.Annotations.prepElement = null;
          archetype._parentProps = c._parentProps;
        }
        else {
          console.warn('no _content');
        }
      }
      else {
        console.warn('no _template');
      }
    },

    // Sets up accessors on the template to call abstract _forwardParentProp
    // API that should be implemented by Templatizer users to get parent
    // properties to their template instances.  These accessors are memoized
    // on the archetype and copied to instances.
    _prepParentProperties: function(archetype) {
      var parentProps = this._parentProps = archetype._parentProps;
      if (this._forwardParentProp && parentProps) {
        // Prototype setup (memoized on archetype)
        var proto = archetype._parentPropProto;
        if (!proto) {
          proto = archetype._parentPropProto = Object.create(null);
          if (this._templatized != this) {
            // Assumption: if `this` isn't the template being templatized,
            // assume that the template is not a Poylmer.Base, so prep it
            // for binding
            Polymer.Bind.prepareModel(proto);
          }
          // Create accessors for each parent prop that forward the property
          // to template instances through abstract _forwardParentProp API
          // that should be implemented by Templatizer users
          for (var prop in parentProps) {
            var parentProp = '_parent_' + prop;
            var effects = [{
              kind: 'function',
              effect: { function: this._createForwardPropEffector(prop) }
            }];
            Polymer.Bind._createAccessors(proto, parentProp, effects);
          }
        }
        // Instance setup
        if (this._templatized != this) {
          Polymer.Bind.prepareInstance(this._templatized);
          this._templatized._forwardParentProp =
            this._forwardParentProp.bind(this);
        }
        this._extendTemplate(this._templatized, proto);
      }
    },

    _createForwardPropEffector: function(prop) {
      return function(source, value) {
        this._forwardParentProp(prop, value);
      };
    },

    // Similar to Polymer.Base.extend, but retains any previously set instance
    // values (_propertySet back on instance once accessor is installed)
    _extendTemplate: function(template, proto) {
      Object.getOwnPropertyNames(proto).forEach(function(n) {
        var val = template[n];
        var pd = Object.getOwnPropertyDescriptor(proto, n);
        Object.defineProperty(template, n, pd);
        if (val !== undefined) {
          template._propertySet(n, val);
        }
      });
    },

    _notifyPathImpl: function(path, value) {
      var p = path.match(/([^.]*)\.(([^.]*).*)/);
                          // 'root.sub.path'
      var root = p[1];    // 'root'
      var sub = p[3];     // 'sub'
      var subPath = p[2]; // 'sub.path'
      // Notify host of parent.* path/property changes
      var dataHost = this.dataHost;
      if (root == 'parent') {
        if (sub == subPath) {
          dataHost.dataHost[sub] = value;
        } else {
          dataHost.notifyPath('_parent_' + subPath, value);
        }
      }
      // Extension point for Templatizer sub-classes
      if (dataHost._forwardInstancePath) {
        dataHost._forwardInstancePath.call(dataHost, this, root, subPath, value);
      }
    },

    // Overrides Base notify-path module
    _pathEffector: function(path, value, fromAbove) {
      if (this._forwardParentPath) {
        if (path.indexOf('_parent_') === 0) {
          this._forwardParentPath(path.substring(8), value);
        }
      }
      Polymer.Base._pathEffector.apply(this, arguments);
    },

    _constructorImpl: function(model, host) {
      var rootDataHost = host._getRootDataHost();
      if (rootDataHost) {
        this.listen = rootDataHost.listen.bind(rootDataHost);
        this._rootDataHost = rootDataHost;
      }
      this._setupConfigure(model);
      this._pushHost(host);
      this.root = this.instanceTemplate(this._template);
      this.root.__styleScoped = true;
      this._popHost();
      this._marshalAnnotatedNodes();
      this._marshalInstanceEffects();
      this._marshalAnnotatedListeners();
      this._tryReady();
    },

    _scopeElementClassImpl: function(node, value) {
      var host = this._rootDataHost;
      if (host) {
        return host._scopeElementClass(node, value);
      }
    },

    stamp: function(model) {
      model = model || {};
      if (this._parentProps) {
        // TODO(kschaaf): Maybe this is okay
        // model.parent = this.dataHost;
        model.parent = model.parent || {};
        for (var prop in this._parentProps) {
          model.parent[prop] = this['_parent_' + prop];
        }
      }
      return new this.ctor(model, this);
    }

    // TODO(sorvell): note, using the template as host is ~5-10% faster if
    // elements have no default values.
    // _constructorImpl: function(model, host) {
    //   this._setupConfigure(model);
    //   host._beginHost();
    //   this.root = this.instanceTemplate(this._template);
    //   host._popHost();
    //   this._marshalTemplateContent();
    //   this._marshalAnnotatedNodes();
    //   this._marshalInstanceEffects();
    //   this._marshalAnnotatedListeners();
    //   this._ready();
    // },

    // stamp: function(model) {
    //   return new this.ctor(model, this.dataHost);
    // }


  };

