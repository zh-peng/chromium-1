
(function() {

  var transformer = document.createElement('style', 'x-style');

  var styleProtector = {

    scopeDocumentSheets: function() {
      this.scopeSheets(this.getSheetsToScope());
    },
    
    getSheetsToScope: function() {
      var s$ = document.styleSheets;
      return Array.prototype.filter.call(s$, function(s) {
        return s.ownerNode.hasAttribute(this.SCOPE_ATTR);
      }, this);
    },

    scopeSheets: function(sheets) {
      for (var i=0, l=sheets.length, s; (i<l) && (s=sheets[i]); i++) {
        this.scopeSheet(s);
      }
    },

    scopeSheet: function(sheet) {
      var owner = sheet.ownerNode;
      var cssText = transformer.scopeCssText(this.rulesToCss(sheet.cssRules));
      if (owner.localName === 'link') {
        var style = document.createElement('style');
        style.textContent = cssText;
        owner.parentNode.insertBefore(style, owner);
        owner.parentNode.removeChild(owner);
      } else {
        owner.textContent = cssText;
      }
    },

    rulesToCss: function(cssRules) {
      for (var i=0, css=[]; i < cssRules.length; i++) {
        css.push(cssRules[i].cssText);
      }
      return css.join('\n\n');
    },

    SCOPE_ATTR: 'needs-scoping'
  };

  addEventListener('DOMContentLoaded', styleProtector.scopeDocumentSheets());

})();
