

  Polymer.Bind = {

    // for prototypes (usually)

    prepareModel: function(model) {
      model._propertyEffects = {};
      model._bindListeners = [];
      // TODO(sjmiles): no mixin function?
      var api = this._modelApi;
      for (var n in api) {
        model[n] = api[n];
      }
    },

    _modelApi: {

      _notifyChange: function(property) {
        var eventName = Polymer.CaseMap.camelToDashCase(property) + '-changed';
        // TODO(sjmiles): oops, `fire` doesn't exist at this layer
        this.fire(eventName, {
          value: this[property]
        }, {bubbles: false});
      },

      // TODO(sjmiles): removing _notifyListener from here breaks accessors.html
      // as a standalone lib. This is temporary, as standard/configure.html
      // installs it's own version on Polymer.Base, and we need that to work
      // right now.
      // NOTE: exists as a hook for processing listeners
      /*
      _notifyListener: function(fn, e) {
        // NOTE: pass e.target because e.target can get lost if this function
        // is queued asynchrously
        return fn.call(this, e, e.target);
      },
      */

      _propertySet: function(property, value, effects) {
        var old = this._data[property];
        if (old !== value) {
          this._data[property] = value;
          if (typeof value == 'object') {
            this._clearPath(property);
          }
          if (effects) {
            this._effectEffects(property, value, effects, old);
          }
        }
        return old;
      },

      _effectEffects: function(property, value, effects, old) {
        effects.forEach(function(fx) {
          //console.log(fx);
          var fn = Polymer.Bind[fx.kind + 'Effect'];
          if (fn) {
            fn.call(this, property, value, fx.effect, old);
          }
        }, this);
      },

      _clearPath: function(path) {
        for (var prop in this._data) {
          if (prop.indexOf(path + '.') === 0) {
            this._data[prop] = undefined;
          }
        }
      }

    },

    // a prepared model can acquire effects

    ensurePropertyEffects: function(model, property) {
      var fx = model._propertyEffects[property];
      if (!fx) {
        fx = model._propertyEffects[property] = [];
      }
      return fx;
    },

    addPropertyEffect: function(model, property, kind, effect) {
      var fx = this.ensurePropertyEffects(model, property);
      fx.push({
        kind: kind,
        effect: effect
      });
    },

    createBindings: function(model) {
      //console.group(model.is);
      // map of properties to effects
      var fx$ = model._propertyEffects;
      if (fx$) {
        // for each property with effects
        for (var n in fx$) {
          // array of effects
          var fx = fx$[n];
          // effects have priority
          fx.sort(this._sortPropertyEffects);
          // create accessors
          this._createAccessors(model, n, fx);
        }
      }
      //console.groupEnd();
    },

    _sortPropertyEffects: (function() {
      // TODO(sjmiles): EFFECT_ORDER buried this way is not ideal,
      // but presumably the sort method is going to be a hot path and not
      // have a `this`. There is also a problematic dependency on effect.kind
      // values here, which are otherwise pluggable.
      var EFFECT_ORDER = {
        'compute': 0,
        'annotation': 1,
        'computedAnnotation': 2,
        'reflect': 3,
        'notify': 4,
        'observer': 5,
        'complexObserver': 6,
        'function': 7
      };
      return function(a, b) {
        return EFFECT_ORDER[a.kind] - EFFECT_ORDER[b.kind];
      };
    })(),

    // create accessors that implement effects

    _createAccessors: function(model, property, effects) {
      var defun = {
        get: function() {
          // TODO(sjmiles): elide delegation for performance, good ROI?
          return this._data[property];
        }
      };
      var setter = function(value) {
        this._propertySet(property, value, effects);
      };
      // ReadOnly properties have a private setter only
      // TODO(kschaaf): Per current Bind factoring, we shouldn't
      // be interrogating the prototype here
      if (model.isReadOnlyProperty && model.isReadOnlyProperty(property)) {
        //model['_' + property + 'Setter'] = setter;
        //model['_set_' + property] = setter;
        model['_set' + this.upper(property)] = setter;
      } else {
        defun.set = setter;
      }
      Object.defineProperty(model, property, defun);
    },

    upper: function(name) {
      return name[0].toUpperCase() + name.substring(1);
    },

    _addAnnotatedListener: function(model, index, property, path, event) {
      var fn = this._notedListenerFactory(property, path,
        this._isStructured(path), this._isEventBogus);
      var eventName = event ||
        (Polymer.CaseMap.camelToDashCase(property) + '-changed');
      model._bindListeners.push({
        index: index,
        property: property,
        path: path,
        changedFn: fn,
        event: eventName
      });
    },

    _isStructured: function(path) {
      return path.indexOf('.') > 0;
    },

    _isEventBogus: function(e, target) {
      return e.path && e.path[0] !== target;
    },

    _notedListenerFactory: function(property, path, isStructured, bogusTest) {
      return function(e, target) {
        if (!bogusTest(e, target)) {
          if (e.detail && e.detail.path) {
            this.notifyPath(this._fixPath(path, property, e.detail.path),
              e.detail.value);
          } else {
            var value = target[property];
            if (!isStructured) {
              this[path] = target[property];
            } else {
              // TODO(kschaaf): dirty check avoids null references when the object has gone away
              if (this._data[path] != value) {
                this.setPathValue(path, value);
              }
            }
          }
        }
      };
    },

    // for instances

    prepareInstance: function(inst) {
      inst._data = Object.create(null);
    },

    setupBindListeners: function(inst) {
      inst._bindListeners.forEach(function(info) {
        // Property listeners:
        // <node>.on.<property>-changed: <path]> = e.detail.value
        //console.log('[_setupBindListener]: [%s][%s] listening for [%s][%s-changed]', this.localName, info.path, info.id || info.index, info.property);
        var node = inst._nodes[info.index];
        node.addEventListener(info.event, inst._notifyListener.bind(inst, info.changedFn));
      });
    }

  };

