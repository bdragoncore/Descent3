import ks from "react";
import rf from "react-dom";
var Js = {};
/**
 * @license React
 * react-dom-server-legacy.browser.production.js
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
var kf;
function _f() {
  if (kf) return Js;
  kf = 1;
  var Te = ks, Vn = rf;
  function G(i) {
    var o = "https://react.dev/errors/" + i;
    if (1 < arguments.length) {
      o += "?args[]=" + encodeURIComponent(arguments[1]);
      for (var f = 2; f < arguments.length; f++)
        o += "&args[]=" + encodeURIComponent(arguments[f]);
    }
    return "Minified React error #" + i + "; visit " + o + " for the full message or use the non-minified dev environment for full errors and additional helpful warnings.";
  }
  var vt = Symbol.for("react.transitional.element"), Qn = Symbol.for("react.portal"), kn = Symbol.for("react.fragment"), Qe = Symbol.for("react.strict_mode"), Kn = Symbol.for("react.profiler"), Nl = Symbol.for("react.consumer"), Et = Symbol.for("react.context"), jt = Symbol.for("react.forward_ref"), H = Symbol.for("react.suspense"), pe = Symbol.for("react.suspense_list"), Ar = Symbol.for("react.memo"), ne = Symbol.for("react.lazy"), N = Symbol.for("react.scope"), Zr = Symbol.for("react.activity"), zo = Symbol.for("react.legacy_hidden"), _i = Symbol.for("react.memo_cache_sentinel"), Bo = Symbol.for("react.view_transition"), Di = Symbol.iterator;
  function Nt(i) {
    return i === null || typeof i != "object" ? null : (i = Di && i[Di] || i["@@iterator"], typeof i == "function" ? i : null);
  }
  var Ee = Array.isArray;
  function Ir(i, o) {
    var f = i.length & 3, g = i.length - f, T = o;
    for (o = 0; o < g; ) {
      var R = i.charCodeAt(o) & 255 | (i.charCodeAt(++o) & 255) << 8 | (i.charCodeAt(++o) & 255) << 16 | (i.charCodeAt(++o) & 255) << 24;
      ++o, R = 3432918353 * (R & 65535) + ((3432918353 * (R >>> 16) & 65535) << 16) & 4294967295, R = R << 15 | R >>> 17, R = 461845907 * (R & 65535) + ((461845907 * (R >>> 16) & 65535) << 16) & 4294967295, T ^= R, T = T << 13 | T >>> 19, T = 5 * (T & 65535) + ((5 * (T >>> 16) & 65535) << 16) & 4294967295, T = (T & 65535) + 27492 + (((T >>> 16) + 58964 & 65535) << 16);
    }
    switch (R = 0, f) {
      case 3:
        R ^= (i.charCodeAt(o + 2) & 255) << 16;
      case 2:
        R ^= (i.charCodeAt(o + 1) & 255) << 8;
      case 1:
        R ^= i.charCodeAt(o) & 255, R = 3432918353 * (R & 65535) + ((3432918353 * (R >>> 16) & 65535) << 16) & 4294967295, R = R << 15 | R >>> 17, T ^= 461845907 * (R & 65535) + ((461845907 * (R >>> 16) & 65535) << 16) & 4294967295;
    }
    return T ^= i.length, T ^= T >>> 16, T = 2246822507 * (T & 65535) + ((2246822507 * (T >>> 16) & 65535) << 16) & 4294967295, T ^= T >>> 13, T = 3266489909 * (T & 65535) + ((3266489909 * (T >>> 16) & 65535) << 16) & 4294967295, (T ^ T >>> 16) >>> 0;
  }
  var Rt = Object.assign, dn = Object.prototype.hasOwnProperty, bt = RegExp(
    "^[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
  ), kc = {}, eo = {};
  function Li(i) {
    return dn.call(eo, i) ? !0 : dn.call(kc, i) ? !1 : bt.test(i) ? eo[i] = !0 : (kc[i] = !0, !1);
  }
  var Wt = new Set(
    "animationIterationCount aspectRatio borderImageOutset borderImageSlice borderImageWidth boxFlex boxFlexGroup boxOrdinalGroup columnCount columns flex flexGrow flexPositive flexShrink flexNegative flexOrder gridArea gridRow gridRowEnd gridRowSpan gridRowStart gridColumn gridColumnEnd gridColumnSpan gridColumnStart fontWeight lineClamp lineHeight opacity order orphans scale tabSize widows zIndex zoom fillOpacity floodOpacity stopOpacity strokeDasharray strokeDashoffset strokeMiterlimit strokeOpacity strokeWidth MozAnimationIterationCount MozBoxFlex MozBoxFlexGroup MozLineClamp msAnimationIterationCount msFlex msZoom msFlexGrow msFlexNegative msFlexOrder msFlexPositive msFlexShrink msGridColumn msGridColumnSpan msGridRow msGridRowSpan WebkitAnimationIterationCount WebkitBoxFlex WebKitBoxFlexGroup WebkitBoxOrdinalGroup WebkitColumnCount WebkitColumns WebkitFlex WebkitFlexGrow WebkitFlexPositive WebkitFlexShrink WebkitLineClamp".split(
      " "
    )
  ), ge = /* @__PURE__ */ new Map([
    ["acceptCharset", "accept-charset"],
    ["htmlFor", "for"],
    ["httpEquiv", "http-equiv"],
    ["crossOrigin", "crossorigin"],
    ["accentHeight", "accent-height"],
    ["alignmentBaseline", "alignment-baseline"],
    ["arabicForm", "arabic-form"],
    ["baselineShift", "baseline-shift"],
    ["capHeight", "cap-height"],
    ["clipPath", "clip-path"],
    ["clipRule", "clip-rule"],
    ["colorInterpolation", "color-interpolation"],
    ["colorInterpolationFilters", "color-interpolation-filters"],
    ["colorProfile", "color-profile"],
    ["colorRendering", "color-rendering"],
    ["dominantBaseline", "dominant-baseline"],
    ["enableBackground", "enable-background"],
    ["fillOpacity", "fill-opacity"],
    ["fillRule", "fill-rule"],
    ["floodColor", "flood-color"],
    ["floodOpacity", "flood-opacity"],
    ["fontFamily", "font-family"],
    ["fontSize", "font-size"],
    ["fontSizeAdjust", "font-size-adjust"],
    ["fontStretch", "font-stretch"],
    ["fontStyle", "font-style"],
    ["fontVariant", "font-variant"],
    ["fontWeight", "font-weight"],
    ["glyphName", "glyph-name"],
    ["glyphOrientationHorizontal", "glyph-orientation-horizontal"],
    ["glyphOrientationVertical", "glyph-orientation-vertical"],
    ["horizAdvX", "horiz-adv-x"],
    ["horizOriginX", "horiz-origin-x"],
    ["imageRendering", "image-rendering"],
    ["letterSpacing", "letter-spacing"],
    ["lightingColor", "lighting-color"],
    ["markerEnd", "marker-end"],
    ["markerMid", "marker-mid"],
    ["markerStart", "marker-start"],
    ["overlinePosition", "overline-position"],
    ["overlineThickness", "overline-thickness"],
    ["paintOrder", "paint-order"],
    ["panose-1", "panose-1"],
    ["pointerEvents", "pointer-events"],
    ["renderingIntent", "rendering-intent"],
    ["shapeRendering", "shape-rendering"],
    ["stopColor", "stop-color"],
    ["stopOpacity", "stop-opacity"],
    ["strikethroughPosition", "strikethrough-position"],
    ["strikethroughThickness", "strikethrough-thickness"],
    ["strokeDasharray", "stroke-dasharray"],
    ["strokeDashoffset", "stroke-dashoffset"],
    ["strokeLinecap", "stroke-linecap"],
    ["strokeLinejoin", "stroke-linejoin"],
    ["strokeMiterlimit", "stroke-miterlimit"],
    ["strokeOpacity", "stroke-opacity"],
    ["strokeWidth", "stroke-width"],
    ["textAnchor", "text-anchor"],
    ["textDecoration", "text-decoration"],
    ["textRendering", "text-rendering"],
    ["transformOrigin", "transform-origin"],
    ["underlinePosition", "underline-position"],
    ["underlineThickness", "underline-thickness"],
    ["unicodeBidi", "unicode-bidi"],
    ["unicodeRange", "unicode-range"],
    ["unitsPerEm", "units-per-em"],
    ["vAlphabetic", "v-alphabetic"],
    ["vHanging", "v-hanging"],
    ["vIdeographic", "v-ideographic"],
    ["vMathematical", "v-mathematical"],
    ["vectorEffect", "vector-effect"],
    ["vertAdvY", "vert-adv-y"],
    ["vertOriginX", "vert-origin-x"],
    ["vertOriginY", "vert-origin-y"],
    ["wordSpacing", "word-spacing"],
    ["writingMode", "writing-mode"],
    ["xmlnsXlink", "xmlns:xlink"],
    ["xHeight", "x-height"]
  ]), V = /["'&<>]/;
  function ue(i) {
    if (typeof i == "boolean" || typeof i == "number" || typeof i == "bigint")
      return "" + i;
    i = "" + i;
    var o = V.exec(i);
    if (o) {
      var f = "", g, T = 0;
      for (g = o.index; g < i.length; g++) {
        switch (i.charCodeAt(g)) {
          case 34:
            o = "&quot;";
            break;
          case 38:
            o = "&amp;";
            break;
          case 39:
            o = "&#x27;";
            break;
          case 60:
            o = "&lt;";
            break;
          case 62:
            o = "&gt;";
            break;
          default:
            continue;
        }
        T !== g && (f += i.slice(T, g)), T = g + 1, f += o;
      }
      i = T !== g ? f + i.slice(T, g) : f;
    }
    return i;
  }
  var Wl = /([A-Z])/g, no = /^ms-/, le = /^[\u0000-\u001F ]*j[\r\n\t]*a[\r\n\t]*v[\r\n\t]*a[\r\n\t]*s[\r\n\t]*c[\r\n\t]*r[\r\n\t]*i[\r\n\t]*p[\r\n\t]*t[\r\n\t]*:/i;
  function F(i) {
    return le.test("" + i) ? "javascript:throw new Error('React has blocked a javascript: URL as a security precaution.')" : i;
  }
  var de = Te.__CLIENT_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, We = Vn.__DOM_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, Ze = {
    pending: !1,
    data: null,
    method: null,
    action: null
  }, He = We.d;
  We.d = {
    f: He.f,
    r: He.r,
    D: Cl,
    C: Ut,
    L: xa,
    m: mc,
    X: Qu,
    S: er,
    M: yu
  };
  var Ct = [], _e = null, pl = /(<\/|<)(s)(cript)/gi;
  function Mr(i, o, f, g) {
    return "" + o + (f === "s" ? "\\u0073" : "\\u0053") + g;
  }
  function Mt(i, o, f, g, T) {
    return {
      idPrefix: i === void 0 ? "" : i,
      nextFormID: 0,
      streamingFormat: 0,
      bootstrapScriptContent: f,
      bootstrapScripts: g,
      bootstrapModules: T,
      instructions: 0,
      hasBody: !1,
      hasHtml: !1,
      unknownResources: {},
      dnsResources: {},
      connectResources: { default: {}, anonymous: {}, credentials: {} },
      imageResources: {},
      styleResources: {},
      scriptResources: {},
      moduleUnknownResources: {},
      moduleScriptResources: {}
    };
  }
  function rt(i, o, f, g) {
    return {
      insertionMode: i,
      selectedValue: o,
      tagScope: f,
      viewTransition: g
    };
  }
  function Sc(i, o, f) {
    var g = i.tagScope & -25;
    switch (o) {
      case "noscript":
        return rt(2, null, g | 1, null);
      case "select":
        return rt(
          2,
          f.value != null ? f.value : f.defaultValue,
          g,
          null
        );
      case "svg":
        return rt(4, null, g, null);
      case "picture":
        return rt(2, null, g | 2, null);
      case "math":
        return rt(5, null, g, null);
      case "foreignObject":
        return rt(2, null, g, null);
      case "table":
        return rt(6, null, g, null);
      case "thead":
      case "tbody":
      case "tfoot":
        return rt(7, null, g, null);
      case "colgroup":
        return rt(9, null, g, null);
      case "tr":
        return rt(8, null, g, null);
      case "head":
        if (2 > i.insertionMode)
          return rt(3, null, g, null);
        break;
      case "html":
        if (i.insertionMode === 0)
          return rt(1, null, g, null);
    }
    return 6 <= i.insertionMode || 2 > i.insertionMode ? rt(2, null, g, null) : i.tagScope !== g ? rt(
      i.insertionMode,
      i.selectedValue,
      g,
      null
    ) : i;
  }
  function Fe(i) {
    return i === null ? null : {
      update: i.update,
      enter: "none",
      exit: "none",
      share: i.update,
      name: i.autoName,
      autoName: i.autoName,
      nameIdx: 0
    };
  }
  function lt(i, o) {
    return o.tagScope & 32 && (i.instructions |= 128), rt(
      o.insertionMode,
      o.selectedValue,
      o.tagScope | 12,
      Fe(o.viewTransition)
    );
  }
  function un(i, o) {
    i = Fe(o.viewTransition);
    var f = o.tagScope | 16;
    return i !== null && i.share !== "none" && (f |= 64), rt(
      o.insertionMode,
      o.selectedValue,
      f,
      i
    );
  }
  var kt = /* @__PURE__ */ new Map();
  function da(i, o) {
    if (typeof o != "object") throw Error(G(62));
    var f = !0, g;
    for (g in o)
      if (dn.call(o, g)) {
        var T = o[g];
        if (T != null && typeof T != "boolean" && T !== "") {
          if (g.indexOf("--") === 0) {
            var R = ue(g);
            T = ue(("" + T).trim());
          } else
            R = kt.get(g), R === void 0 && (R = ue(
              g.replace(Wl, "-$1").toLowerCase().replace(no, "-ms-")
            ), kt.set(g, R)), T = typeof T == "number" ? T === 0 || Wt.has(g) ? "" + T : T + "px" : ue(("" + T).trim());
          f ? (f = !1, i.push(' style="', R, ":", T)) : i.push(";", R, ":", T);
        }
      }
    f || i.push('"');
  }
  function zi(i, o, f) {
    f && typeof f != "function" && typeof f != "symbol" && i.push(" ", o, '=""');
  }
  function Ot(i, o, f) {
    typeof f != "function" && typeof f != "symbol" && typeof f != "boolean" && i.push(" ", o, '="', ue(f), '"');
  }
  var xr = ue(
    "javascript:throw new Error('React form unexpectedly submitted.')"
  );
  function El(i, o) {
    this.push('<input type="hidden"'), yt(i), Ot(this, "name", o), Ot(this, "value", i), this.push("/>");
  }
  function yt(i) {
    if (typeof i != "string") throw Error(G(480));
  }
  function Rl(i, o) {
    if (typeof o.$$FORM_ACTION == "function") {
      var f = i.nextFormID++;
      i = i.idPrefix + f;
      try {
        var g = o.$$FORM_ACTION(i);
        if (g) {
          var T = g.data;
          T != null && T.forEach(yt);
        }
        return g;
      } catch (R) {
        if (typeof R == "object" && R !== null && typeof R.then == "function")
          throw R;
      }
    }
    return null;
  }
  function Pc(i, o, f, g, T, R, S, Y) {
    var I = null;
    if (typeof g == "function") {
      var W = Rl(o, g);
      W !== null ? (Y = W.name, g = W.action || "", T = W.encType, R = W.method, S = W.target, I = W.data) : (i.push(" ", "formAction", '="', xr, '"'), S = R = T = g = Y = null, Xu(o, f));
    }
    return Y != null && qe(i, "name", Y), g != null && qe(i, "formAction", g), T != null && qe(i, "formEncType", T), R != null && qe(i, "formMethod", R), S != null && qe(i, "formTarget", S), I;
  }
  function qe(i, o, f) {
    switch (o) {
      case "className":
        Ot(i, "class", f);
        break;
      case "tabIndex":
        Ot(i, "tabindex", f);
        break;
      case "dir":
      case "role":
      case "viewBox":
      case "width":
      case "height":
        Ot(i, o, f);
        break;
      case "style":
        da(i, f);
        break;
      case "src":
      case "href":
        if (f === "") break;
      case "action":
      case "formAction":
        if (f == null || typeof f == "function" || typeof f == "symbol" || typeof f == "boolean")
          break;
        f = F("" + f), i.push(" ", o, '="', ue(f), '"');
        break;
      case "defaultValue":
      case "defaultChecked":
      case "innerHTML":
      case "suppressContentEditableWarning":
      case "suppressHydrationWarning":
      case "ref":
        break;
      case "autoFocus":
      case "multiple":
      case "muted":
        zi(i, o.toLowerCase(), f);
        break;
      case "xlinkHref":
        if (typeof f == "function" || typeof f == "symbol" || typeof f == "boolean")
          break;
        f = F("" + f), i.push(" ", "xlink:href", '="', ue(f), '"');
        break;
      case "contentEditable":
      case "spellCheck":
      case "draggable":
      case "value":
      case "autoReverse":
      case "externalResourcesRequired":
      case "focusable":
      case "preserveAlpha":
        typeof f != "function" && typeof f != "symbol" && i.push(" ", o, '="', ue(f), '"');
        break;
      case "inert":
      case "allowFullScreen":
      case "async":
      case "autoPlay":
      case "controls":
      case "default":
      case "defer":
      case "disabled":
      case "disablePictureInPicture":
      case "disableRemotePlayback":
      case "formNoValidate":
      case "hidden":
      case "loop":
      case "noModule":
      case "noValidate":
      case "open":
      case "playsInline":
      case "readOnly":
      case "required":
      case "reversed":
      case "scoped":
      case "seamless":
      case "itemScope":
        f && typeof f != "function" && typeof f != "symbol" && i.push(" ", o, '=""');
        break;
      case "capture":
      case "download":
        f === !0 ? i.push(" ", o, '=""') : f !== !1 && typeof f != "function" && typeof f != "symbol" && i.push(" ", o, '="', ue(f), '"');
        break;
      case "cols":
      case "rows":
      case "size":
      case "span":
        typeof f != "function" && typeof f != "symbol" && !isNaN(f) && 1 <= f && i.push(" ", o, '="', ue(f), '"');
        break;
      case "rowSpan":
      case "start":
        typeof f == "function" || typeof f == "symbol" || isNaN(f) || i.push(" ", o, '="', ue(f), '"');
        break;
      case "xlinkActuate":
        Ot(i, "xlink:actuate", f);
        break;
      case "xlinkArcrole":
        Ot(i, "xlink:arcrole", f);
        break;
      case "xlinkRole":
        Ot(i, "xlink:role", f);
        break;
      case "xlinkShow":
        Ot(i, "xlink:show", f);
        break;
      case "xlinkTitle":
        Ot(i, "xlink:title", f);
        break;
      case "xlinkType":
        Ot(i, "xlink:type", f);
        break;
      case "xmlBase":
        Ot(i, "xml:base", f);
        break;
      case "xmlLang":
        Ot(i, "xml:lang", f);
        break;
      case "xmlSpace":
        Ot(i, "xml:space", f);
        break;
      default:
        if ((!(2 < o.length) || o[0] !== "o" && o[0] !== "O" || o[1] !== "n" && o[1] !== "N") && (o = ge.get(o) || o, Li(o))) {
          switch (typeof f) {
            case "function":
            case "symbol":
              return;
            case "boolean":
              var g = o.toLowerCase().slice(0, 5);
              if (g !== "data-" && g !== "aria-") return;
          }
          i.push(" ", o, '="', ue(f), '"');
        }
    }
  }
  function $t(i, o, f) {
    if (o != null) {
      if (f != null) throw Error(G(60));
      if (typeof o != "object" || !("__html" in o))
        throw Error(G(61));
      o = o.__html, o != null && i.push("" + o);
    }
  }
  function Rn(i) {
    var o = "";
    return Te.Children.forEach(i, function(f) {
      f != null && (o += f);
    }), o;
  }
  function Xu(i, o) {
    if (!(i.instructions & 16)) {
      i.instructions |= 16;
      var f = o.preamble, g = o.bootstrapChunks;
      (f.htmlChunks || f.headChunks) && g.length === 0 ? (g.push(o.startInlineScript), Go(g, i), g.push(
        ">",
        `addEventListener("submit",function(a){if(!a.defaultPrevented){var c=a.target,d=a.submitter,e=c.action,b=d;if(d){var f=d.getAttribute("formAction");null!=f&&(e=f,b=null)}"javascript:throw new Error('React form unexpectedly submitted.')"===e&&(a.preventDefault(),b?(a=document.createElement("input"),a.name=b.name,a.value=b.value,b.parentNode.insertBefore(a,b),b=new FormData(c),a.parentNode.removeChild(a)):b=new FormData(c),a=c.ownerDocument||c,(a.$$reactFormReplay=a.$$reactFormReplay||[]).push(c,d,b))}});`,
        "<\/script>"
      )) : g.unshift(
        o.startInlineScript,
        ">",
        `addEventListener("submit",function(a){if(!a.defaultPrevented){var c=a.target,d=a.submitter,e=c.action,b=d;if(d){var f=d.getAttribute("formAction");null!=f&&(e=f,b=null)}"javascript:throw new Error('React form unexpectedly submitted.')"===e&&(a.preventDefault(),b?(a=document.createElement("input"),a.name=b.name,a.value=b.value,b.parentNode.insertBefore(a,b),b=new FormData(c),a.parentNode.removeChild(a)):b=new FormData(c),a=c.ownerDocument||c,(a.$$reactFormReplay=a.$$reactFormReplay||[]).push(c,d,b))}});`,
        "<\/script>"
      );
    }
  }
  function et(i, o) {
    i.push(Nn("link"));
    for (var f in o)
      if (dn.call(o, f)) {
        var g = o[f];
        if (g != null)
          switch (f) {
            case "children":
            case "dangerouslySetInnerHTML":
              throw Error(G(399, "link"));
            default:
              qe(i, f, g);
          }
      }
    return i.push("/>"), null;
  }
  var hu = /(<\/|<)(s)(tyle)/gi;
  function gu(i, o, f, g) {
    return "" + o + (f === "s" ? "\\73 " : "\\53 ") + g;
  }
  function si(i, o, f) {
    i.push(Nn(f));
    for (var g in o)
      if (dn.call(o, g)) {
        var T = o[g];
        if (T != null)
          switch (g) {
            case "children":
            case "dangerouslySetInnerHTML":
              throw Error(G(399, f));
            default:
              qe(i, g, T);
          }
      }
    return i.push("/>"), null;
  }
  function vu(i, o) {
    i.push(Nn("title"));
    var f = null, g = null, T;
    for (T in o)
      if (dn.call(o, T)) {
        var R = o[T];
        if (R != null)
          switch (T) {
            case "children":
              f = R;
              break;
            case "dangerouslySetInnerHTML":
              g = R;
              break;
            default:
              qe(i, T, R);
          }
      }
    return i.push(">"), o = Array.isArray(f) ? 2 > f.length ? f[0] : null : f, typeof o != "function" && typeof o != "symbol" && o !== null && o !== void 0 && i.push(ue("" + o)), $t(i, g, f), i.push(va("title")), null;
  }
  function No(i, o) {
    i.push(Nn("script"));
    var f = null, g = null, T;
    for (T in o)
      if (dn.call(o, T)) {
        var R = o[T];
        if (R != null)
          switch (T) {
            case "children":
              f = R;
              break;
            case "dangerouslySetInnerHTML":
              g = R;
              break;
            default:
              qe(i, T, R);
          }
      }
    return i.push(">"), $t(i, g, f), typeof f == "string" && i.push(("" + f).replace(pl, Mr)), i.push(va("script")), null;
  }
  function ha(i, o, f) {
    i.push(Nn(f));
    var g = f = null, T;
    for (T in o)
      if (dn.call(o, T)) {
        var R = o[T];
        if (R != null)
          switch (T) {
            case "children":
              f = R;
              break;
            case "dangerouslySetInnerHTML":
              g = R;
              break;
            default:
              qe(i, T, R);
          }
      }
    return i.push(">"), $t(i, g, f), f;
  }
  function Bi(i, o, f) {
    i.push(Nn(f));
    var g = f = null, T;
    for (T in o)
      if (dn.call(o, T)) {
        var R = o[T];
        if (R != null)
          switch (T) {
            case "children":
              f = R;
              break;
            case "dangerouslySetInnerHTML":
              g = R;
              break;
            default:
              qe(i, T, R);
          }
      }
    return i.push(">"), $t(i, g, f), typeof f == "string" ? (i.push(ue(f)), null) : f;
  }
  var ga = /^[a-zA-Z][a-zA-Z:_\.\-\d]*$/, Wo = /* @__PURE__ */ new Map();
  function Nn(i) {
    var o = Wo.get(i);
    if (o === void 0) {
      if (!ga.test(i))
        throw Error(G(65, i));
      o = "<" + i, Wo.set(i, o);
    }
    return o;
  }
  function An(i, o, f, g, T, R, S, Y, I) {
    switch (o) {
      case "div":
      case "span":
      case "svg":
      case "path":
        break;
      case "a":
        i.push(Nn("a"));
        var W = null, q = null, Q;
        for (Q in f)
          if (dn.call(f, Q)) {
            var ae = f[Q];
            if (ae != null)
              switch (Q) {
                case "children":
                  W = ae;
                  break;
                case "dangerouslySetInnerHTML":
                  q = ae;
                  break;
                case "href":
                  ae === "" ? Ot(i, "href", "") : qe(i, Q, ae);
                  break;
                default:
                  qe(i, Q, ae);
              }
          }
        if (i.push(">"), $t(i, q, W), typeof W == "string") {
          i.push(ue(W));
          var ve = null;
        } else ve = W;
        return ve;
      case "g":
      case "p":
      case "li":
        break;
      case "select":
        i.push(Nn("select"));
        var De = null, ke = null, xe;
        for (xe in f)
          if (dn.call(f, xe)) {
            var me = f[xe];
            if (me != null)
              switch (xe) {
                case "children":
                  De = me;
                  break;
                case "dangerouslySetInnerHTML":
                  ke = me;
                  break;
                case "defaultValue":
                case "value":
                  break;
                default:
                  qe(
                    i,
                    xe,
                    me
                  );
              }
          }
        return i.push(">"), $t(i, ke, De), De;
      case "option":
        var Ce = Y.selectedValue;
        i.push(Nn("option"));
        var _n = null, Le = null, Je = null, on = null, Me;
        for (Me in f)
          if (dn.call(f, Me)) {
            var Hn = f[Me];
            if (Hn != null)
              switch (Me) {
                case "children":
                  _n = Hn;
                  break;
                case "selected":
                  Je = Hn;
                  break;
                case "dangerouslySetInnerHTML":
                  on = Hn;
                  break;
                case "value":
                  Le = Hn;
                default:
                  qe(
                    i,
                    Me,
                    Hn
                  );
              }
          }
        if (Ce != null) {
          var nn = Le !== null ? "" + Le : Rn(_n);
          if (Ee(Ce)) {
            for (var nr = 0; nr < Ce.length; nr++)
              if ("" + Ce[nr] === nn) {
                i.push(' selected=""');
                break;
              }
          } else
            "" + Ce === nn && i.push(' selected=""');
        } else Je && i.push(' selected=""');
        return i.push(">"), $t(i, on, _n), _n;
      case "textarea":
        i.push(Nn("textarea"));
        var sn = null, Ke = null, tn = null, Ae;
        for (Ae in f)
          if (dn.call(f, Ae)) {
            var rn = f[Ae];
            if (rn != null)
              switch (Ae) {
                case "children":
                  tn = rn;
                  break;
                case "value":
                  sn = rn;
                  break;
                case "defaultValue":
                  Ke = rn;
                  break;
                case "dangerouslySetInnerHTML":
                  throw Error(G(91));
                default:
                  qe(
                    i,
                    Ae,
                    rn
                  );
              }
          }
        if (sn === null && Ke !== null && (sn = Ke), i.push(">"), tn != null) {
          if (sn != null) throw Error(G(92));
          if (Ee(tn)) {
            if (1 < tn.length)
              throw Error(G(93));
            sn = "" + tn[0];
          }
          sn = "" + tn;
        }
        return typeof sn == "string" && sn[0] === `
` && i.push(`
`), sn !== null && i.push(ue("" + sn)), null;
      case "input":
        i.push(Nn("input"));
        var Ft = null, Be = null, qi = null, Ei = null, tr = null, il = null, al = null, ol = null, ql = null, Ri;
        for (Ri in f)
          if (dn.call(f, Ri)) {
            var wt = f[Ri];
            if (wt != null)
              switch (Ri) {
                case "children":
                case "dangerouslySetInnerHTML":
                  throw Error(G(399, "input"));
                case "name":
                  Ft = wt;
                  break;
                case "formAction":
                  Be = wt;
                  break;
                case "formEncType":
                  qi = wt;
                  break;
                case "formMethod":
                  Ei = wt;
                  break;
                case "formTarget":
                  tr = wt;
                  break;
                case "defaultChecked":
                  ql = wt;
                  break;
                case "defaultValue":
                  al = wt;
                  break;
                case "checked":
                  ol = wt;
                  break;
                case "value":
                  il = wt;
                  break;
                default:
                  qe(
                    i,
                    Ri,
                    wt
                  );
              }
          }
        var Aa = Pc(
          i,
          g,
          T,
          Be,
          qi,
          Ei,
          tr,
          Ft
        );
        return ol !== null ? zi(i, "checked", ol) : ql !== null && zi(i, "checked", ql), il !== null ? qe(i, "value", il) : al !== null && qe(i, "value", al), i.push("/>"), Aa != null && Aa.forEach(El, i), null;
      case "button":
        i.push(Nn("button"));
        var Ia = null, lc = null, Hc = null, bo = null, ji = null, rr = null, ic = null, lr;
        for (lr in f)
          if (dn.call(f, lr)) {
            var $r = f[lr];
            if ($r != null)
              switch (lr) {
                case "children":
                  Ia = $r;
                  break;
                case "dangerouslySetInnerHTML":
                  lc = $r;
                  break;
                case "name":
                  Hc = $r;
                  break;
                case "formAction":
                  bo = $r;
                  break;
                case "formEncType":
                  ji = $r;
                  break;
                case "formMethod":
                  rr = $r;
                  break;
                case "formTarget":
                  ic = $r;
                  break;
                default:
                  qe(
                    i,
                    lr,
                    $r
                  );
              }
          }
        var Ci = Pc(
          i,
          g,
          T,
          bo,
          ji,
          rr,
          ic,
          Hc
        );
        if (i.push(">"), Ci != null && Ci.forEach(El, i), $t(i, lc, Ia), typeof Ia == "string") {
          i.push(ue(Ia));
          var ac = null;
        } else ac = Ia;
        return ac;
      case "form":
        i.push(Nn("form"));
        var ki = null, Uc = null, ct = null, oc = null, $i = null, yo = null, Ma;
        for (Ma in f)
          if (dn.call(f, Ma)) {
            var nt = f[Ma];
            if (nt != null)
              switch (Ma) {
                case "children":
                  ki = nt;
                  break;
                case "dangerouslySetInnerHTML":
                  Uc = nt;
                  break;
                case "action":
                  ct = nt;
                  break;
                case "encType":
                  oc = nt;
                  break;
                case "method":
                  $i = nt;
                  break;
                case "target":
                  yo = nt;
                  break;
                default:
                  qe(
                    i,
                    Ma,
                    nt
                  );
              }
          }
        var el = null, ze = null;
        if (typeof ct == "function") {
          var Si = Rl(
            g,
            ct
          );
          Si !== null ? (ct = Si.action || "", oc = Si.encType, $i = Si.method, yo = Si.target, el = Si.data, ze = Si.name) : (i.push(
            " ",
            "action",
            '="',
            xr,
            '"'
          ), yo = $i = oc = ct = null, Xu(g, T));
        }
        if (ct != null && qe(i, "action", ct), oc != null && qe(i, "encType", oc), $i != null && qe(i, "method", $i), yo != null && qe(i, "target", yo), i.push(">"), ze !== null && (i.push('<input type="hidden"'), Ot(i, "name", ze), i.push("/>"), el != null && el.forEach(El, i)), $t(i, Uc, ki), typeof ki == "string") {
          i.push(ue(ki));
          var xo = null;
        } else xo = ki;
        return xo;
      case "menuitem":
        i.push(Nn("menuitem"));
        for (var ea in f)
          if (dn.call(f, ea)) {
            var Un = f[ea];
            if (Un != null)
              switch (ea) {
                case "children":
                case "dangerouslySetInnerHTML":
                  throw Error(G(400));
                default:
                  qe(
                    i,
                    ea,
                    Un
                  );
              }
          }
        return i.push(">"), null;
      case "object":
        i.push(Nn("object"));
        var Cr = null, na = null, Oa;
        for (Oa in f)
          if (dn.call(f, Oa)) {
            var cl = f[Oa];
            if (cl != null)
              switch (Oa) {
                case "children":
                  Cr = cl;
                  break;
                case "dangerouslySetInnerHTML":
                  na = cl;
                  break;
                case "data":
                  var Xt = F("" + cl);
                  if (Xt === "") break;
                  i.push(
                    " ",
                    "data",
                    '="',
                    ue(Xt),
                    '"'
                  );
                  break;
                default:
                  qe(
                    i,
                    Oa,
                    cl
                  );
              }
          }
        if (i.push(">"), $t(i, na, Cr), typeof Cr == "string") {
          i.push(ue(Cr));
          var To = null;
        } else To = Cr;
        return To;
      case "title":
        var ir = Y.tagScope & 1, Fu = Y.tagScope & 4;
        if (Y.insertionMode === 4 || ir || f.itemProp != null)
          var _a = vu(
            i,
            f
          );
        else
          Fu ? _a = null : (vu(T.hoistableChunks, f), _a = void 0);
        return _a;
      case "link":
        var cc = Y.tagScope & 1, Yc = Y.tagScope & 4, Gc = f.rel, jl = f.href, Da = f.precedence;
        if (Y.insertionMode === 4 || cc || f.itemProp != null || typeof Gc != "string" || typeof jl != "string" || jl === "") {
          et(i, f);
          var La = null;
        } else if (f.rel === "stylesheet")
          if (typeof Da != "string" || f.disabled != null || f.onLoad || f.onError)
            La = et(
              i,
              f
            );
          else {
            var ar = T.styles.get(Da), uc = g.styleResources.hasOwnProperty(jl) ? g.styleResources[jl] : void 0;
            if (uc !== null) {
              g.styleResources[jl] = null, ar || (ar = {
                precedence: ue(Da),
                rules: [],
                hrefs: [],
                sheets: /* @__PURE__ */ new Map()
              }, T.styles.set(Da, ar));
              var sc = {
                state: 0,
                props: Rt({}, f, {
                  "data-precedence": f.precedence,
                  precedence: null
                })
              };
              if (uc) {
                uc.length === 2 && Ac(sc.props, uc);
                var mu = T.preloads.stylesheets.get(jl);
                mu && 0 < mu.length ? mu.length = 0 : sc.state = 1;
              }
              ar.sheets.set(jl, sc), S && S.stylesheets.add(sc);
            } else if (ar) {
              var Au = ar.sheets.get(jl);
              Au && S && S.stylesheets.add(Au);
            }
            I && i.push("<!-- -->"), La = null;
          }
        else
          f.onLoad || f.onError ? La = et(
            i,
            f
          ) : (I && i.push("<!-- -->"), La = Yc ? null : et(T.hoistableChunks, f));
        return La;
      case "script":
        var Xc = Y.tagScope & 1, Iu = f.async;
        if (typeof f.src != "string" || !f.src || !Iu || typeof Iu == "function" || typeof Iu == "symbol" || f.onLoad || f.onError || Y.insertionMode === 4 || Xc || f.itemProp != null)
          var Mu = No(
            i,
            f
          );
        else {
          var ut = f.src;
          if (f.type === "module")
            var Zc = g.moduleScriptResources, fc = T.preloads.moduleScripts;
          else
            Zc = g.scriptResources, fc = T.preloads.scripts;
          var dc = Zc.hasOwnProperty(ut) ? Zc[ut] : void 0;
          if (dc !== null) {
            Zc[ut] = null;
            var Jc = f;
            if (dc) {
              dc.length === 2 && (Jc = Rt({}, f), Ac(Jc, dc));
              var Ou = fc.get(ut);
              Ou && (Ou.length = 0);
            }
            var Vc = [];
            T.scripts.add(Vc), No(Vc, Jc);
          }
          I && i.push("<!-- -->"), Mu = null;
        }
        return Mu;
      case "style":
        var Qc = Y.tagScope & 1, ta = f.precedence, ul = f.href, ts = f.nonce;
        if (Y.insertionMode === 4 || Qc || f.itemProp != null || typeof ta != "string" || typeof ul != "string" || ul === "") {
          i.push(Nn("style"));
          var za = null, hc = null, Ml;
          for (Ml in f)
            if (dn.call(f, Ml)) {
              var wo = f[Ml];
              if (wo != null)
                switch (Ml) {
                  case "children":
                    za = wo;
                    break;
                  case "dangerouslySetInnerHTML":
                    hc = wo;
                    break;
                  default:
                    qe(
                      i,
                      Ml,
                      wo
                    );
                }
            }
          i.push(">");
          var ra = Array.isArray(za) ? 2 > za.length ? za[0] : null : za;
          typeof ra != "function" && typeof ra != "symbol" && ra !== null && ra !== void 0 && i.push(("" + ra).replace(hu, gu)), $t(i, hc, za), i.push(va("style"));
          var gc = null;
        } else {
          var la = T.styles.get(ta);
          if ((g.styleResources.hasOwnProperty(ul) ? g.styleResources[ul] : void 0) !== null) {
            g.styleResources[ul] = null, la || (la = {
              precedence: ue(ta),
              rules: [],
              hrefs: [],
              sheets: /* @__PURE__ */ new Map()
            }, T.styles.set(ta, la));
            var rs = T.nonce.style;
            if (!rs || rs === ts) {
              la.hrefs.push(ue(ul));
              var ls = la.rules, Ol = null, po = null, nl;
              for (nl in f)
                if (dn.call(f, nl)) {
                  var ia = f[nl];
                  if (ia != null)
                    switch (nl) {
                      case "children":
                        Ol = ia;
                        break;
                      case "dangerouslySetInnerHTML":
                        po = ia;
                    }
                }
              var Ba = Array.isArray(Ol) ? 2 > Ol.length ? Ol[0] : null : Ol;
              typeof Ba != "function" && typeof Ba != "symbol" && Ba !== null && Ba !== void 0 && ls.push(
                ("" + Ba).replace(hu, gu)
              ), $t(ls, po, Ol);
            }
          }
          la && S && S.styles.add(la), I && i.push("<!-- -->"), gc = void 0;
        }
        return gc;
      case "meta":
        var sl = Y.tagScope & 1, _u = Y.tagScope & 4;
        if (Y.insertionMode === 4 || sl || f.itemProp != null)
          var is = si(
            i,
            f,
            "meta"
          );
        else
          I && i.push("<!-- -->"), is = _u ? null : typeof f.charSet == "string" ? si(T.charsetChunks, f, "meta") : f.name === "viewport" ? si(T.viewportChunks, f, "meta") : si(T.hoistableChunks, f, "meta");
        return is;
      case "listing":
      case "pre":
        i.push(Nn(o));
        var Eo = null, l = null, a;
        for (a in f)
          if (dn.call(f, a)) {
            var s = f[a];
            if (s != null)
              switch (a) {
                case "children":
                  Eo = s;
                  break;
                case "dangerouslySetInnerHTML":
                  l = s;
                  break;
                default:
                  qe(
                    i,
                    a,
                    s
                  );
              }
          }
        if (i.push(">"), l != null) {
          if (Eo != null) throw Error(G(60));
          if (typeof l != "object" || !("__html" in l))
            throw Error(G(61));
          var v = l.__html;
          v != null && (typeof v == "string" && 0 < v.length && v[0] === `
` ? i.push(`
`, v) : i.push("" + v));
        }
        return typeof Eo == "string" && Eo[0] === `
` && i.push(`
`), Eo;
      case "img":
        var x = Y.tagScope & 3, E = f.src, C = f.srcSet;
        if (!(f.loading === "lazy" || !E && !C || typeof E != "string" && E != null || typeof C != "string" && C != null || f.fetchPriority === "low" || x) && (typeof E != "string" || E[4] !== ":" || E[0] !== "d" && E[0] !== "D" || E[1] !== "a" && E[1] !== "A" || E[2] !== "t" && E[2] !== "T" || E[3] !== "a" && E[3] !== "A") && (typeof C != "string" || C[4] !== ":" || C[0] !== "d" && C[0] !== "D" || C[1] !== "a" && C[1] !== "A" || C[2] !== "t" && C[2] !== "T" || C[3] !== "a" && C[3] !== "A")) {
          S !== null && Y.tagScope & 64 && (S.suspenseyImages = !0);
          var _ = typeof f.sizes == "string" ? f.sizes : void 0, m = C ? C + `
` + (_ || "") : E, D = T.preloads.images, U = D.get(m);
          if (U)
            (f.fetchPriority === "high" || 10 > T.highImagePreloads.size) && (D.delete(m), T.highImagePreloads.add(U));
          else if (!g.imageResources.hasOwnProperty(m)) {
            g.imageResources[m] = Ct;
            var Z = f.crossOrigin, se = typeof Z == "string" ? Z === "use-credentials" ? Z : "" : void 0, ce = T.headers, Ge;
            ce && 0 < ce.remainingCapacity && typeof f.srcSet != "string" && (f.fetchPriority === "high" || 500 > ce.highImagePreloads.length) && (Ge = In(E, "image", {
              imageSrcSet: f.srcSet,
              imageSizes: f.sizes,
              crossOrigin: se,
              integrity: f.integrity,
              nonce: f.nonce,
              type: f.type,
              fetchPriority: f.fetchPriority,
              referrerPolicy: f.refererPolicy
            }), 0 <= (ce.remainingCapacity -= Ge.length + 2)) ? (T.resets.image[m] = Ct, ce.highImagePreloads && (ce.highImagePreloads += ", "), ce.highImagePreloads += Ge) : (U = [], et(U, {
              rel: "preload",
              as: "image",
              href: C ? void 0 : E,
              imageSrcSet: C,
              imageSizes: _,
              crossOrigin: se,
              integrity: f.integrity,
              type: f.type,
              fetchPriority: f.fetchPriority,
              referrerPolicy: f.referrerPolicy
            }), f.fetchPriority === "high" || 10 > T.highImagePreloads.size ? T.highImagePreloads.add(U) : (T.bulkPreloads.add(U), D.set(m, U)));
          }
        }
        return si(i, f, "img");
      case "base":
      case "area":
      case "br":
      case "col":
      case "embed":
      case "hr":
      case "keygen":
      case "param":
      case "source":
      case "track":
      case "wbr":
        return si(i, f, o);
      case "annotation-xml":
      case "color-profile":
      case "font-face":
      case "font-face-src":
      case "font-face-uri":
      case "font-face-format":
      case "font-face-name":
      case "missing-glyph":
        break;
      case "head":
        if (2 > Y.insertionMode) {
          var Se = R || T.preamble;
          if (Se.headChunks)
            throw Error(G(545, "`<head>`"));
          R !== null && i.push("<!--head-->"), Se.headChunks = [];
          var Xe = ha(
            Se.headChunks,
            f,
            "head"
          );
        } else
          Xe = Bi(
            i,
            f,
            "head"
          );
        return Xe;
      case "body":
        if (2 > Y.insertionMode) {
          var Yn = R || T.preamble;
          if (Yn.bodyChunks)
            throw Error(G(545, "`<body>`"));
          R !== null && i.push("<!--body-->"), Yn.bodyChunks = [];
          var Fn = ha(
            Yn.bodyChunks,
            f,
            "body"
          );
        } else
          Fn = Bi(
            i,
            f,
            "body"
          );
        return Fn;
      case "html":
        if (Y.insertionMode === 0) {
          var Zt = R || T.preamble;
          if (Zt.htmlChunks)
            throw Error(G(545, "`<html>`"));
          R !== null && i.push("<!--html-->"), Zt.htmlChunks = [""];
          var Dr = ha(
            Zt.htmlChunks,
            f,
            "html"
          );
        } else
          Dr = Bi(
            i,
            f,
            "html"
          );
        return Dr;
      default:
        if (o.indexOf("-") !== -1) {
          i.push(Nn(o));
          var fl = null, Pe = null, or;
          for (or in f)
            if (dn.call(f, or)) {
              var tt = f[or];
              if (tt != null) {
                var kr = or;
                switch (or) {
                  case "children":
                    fl = tt;
                    break;
                  case "dangerouslySetInnerHTML":
                    Pe = tt;
                    break;
                  case "style":
                    da(i, tt);
                    break;
                  case "suppressContentEditableWarning":
                  case "suppressHydrationWarning":
                  case "ref":
                    break;
                  case "className":
                    kr = "class";
                  default:
                    if (Li(or) && typeof tt != "function" && typeof tt != "symbol" && tt !== !1) {
                      if (tt === !0) tt = "";
                      else if (typeof tt == "object") continue;
                      i.push(
                        " ",
                        kr,
                        '="',
                        ue(tt),
                        '"'
                      );
                    }
                }
              }
            }
          return i.push(">"), $t(i, Pe, fl), fl;
        }
    }
    return Bi(i, f, o);
  }
  var Fc = /* @__PURE__ */ new Map();
  function va(i) {
    var o = Fc.get(i);
    return o === void 0 && (o = "</" + i + ">", Fc.set(i, o)), o;
  }
  function Zu(i, o) {
    i = i.preamble, i.htmlChunks === null && o.htmlChunks && (i.htmlChunks = o.htmlChunks), i.headChunks === null && o.headChunks && (i.headChunks = o.headChunks), i.bodyChunks === null && o.bodyChunks && (i.bodyChunks = o.bodyChunks);
  }
  function bu(i, o) {
    o = o.bootstrapChunks;
    for (var f = 0; f < o.length - 1; f++)
      i.push(o[f]);
    return f < o.length ? (f = o[f], o.length = 0, i.push(f)) : !0;
  }
  function ll(i, o, f) {
    if (i.push('<!--$?--><template id="'), f === null) throw Error(G(395));
    return i.push(o.boundaryPrefix), o = f.toString(16), i.push(o), i.push('"></template>');
  }
  function Hl(i, o, f, g) {
    switch (f.insertionMode) {
      case 0:
      case 1:
      case 3:
      case 2:
        return i.push('<div hidden id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      case 4:
        return i.push('<svg aria-hidden="true" style="display:none" id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      case 5:
        return i.push('<math aria-hidden="true" style="display:none" id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      case 6:
        return i.push('<table hidden id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      case 7:
        return i.push('<table hidden><tbody id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      case 8:
        return i.push('<table hidden><tr id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      case 9:
        return i.push('<table hidden><colgroup id="'), i.push(o.segmentPrefix), o = g.toString(16), i.push(o), i.push('">');
      default:
        throw Error(G(397));
    }
  }
  function Ju(i, o) {
    switch (o.insertionMode) {
      case 0:
      case 1:
      case 3:
      case 2:
        return i.push("</div>");
      case 4:
        return i.push("</svg>");
      case 5:
        return i.push("</math>");
      case 6:
        return i.push("</table>");
      case 7:
        return i.push("</tbody></table>");
      case 8:
        return i.push("</tr></table>");
      case 9:
        return i.push("</colgroup></table>");
      default:
        throw Error(G(397));
    }
  }
  var ba = /[<\u2028\u2029]/g;
  function qn(i) {
    return JSON.stringify(i).replace(
      ba,
      function(o) {
        switch (o) {
          case "<":
            return "\\u003c";
          case "\u2028":
            return "\\u2028";
          case "\u2029":
            return "\\u2029";
          default:
            throw Error(
              "escapeJSStringsForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
            );
        }
      }
    );
  }
  var Ni = /[&><\u2028\u2029]/g;
  function Wi(i) {
    return JSON.stringify(i).replace(
      Ni,
      function(o) {
        switch (o) {
          case "&":
            return "\\u0026";
          case ">":
            return "\\u003e";
          case "<":
            return "\\u003c";
          case "\u2028":
            return "\\u2028";
          case "\u2029":
            return "\\u2029";
          default:
            throw Error(
              "escapeJSObjectForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
            );
        }
      }
    );
  }
  var ya = !1, Ht = !0;
  function Or(i) {
    var o = i.rules, f = i.hrefs, g = 0;
    if (f.length) {
      for (this.push(_e.startInlineStyle), this.push(' media="not all" data-precedence="'), this.push(i.precedence), this.push('" data-href="'); g < f.length - 1; g++)
        this.push(f[g]), this.push(" ");
      for (this.push(f[g]), this.push('">'), g = 0; g < o.length; g++) this.push(o[g]);
      Ht = this.push("</style>"), ya = !0, o.length = 0, f.length = 0;
    }
  }
  function xn(i) {
    return i.state !== 2 ? ya = !0 : !1;
  }
  function Ho(i, o, f) {
    return ya = !1, Ht = !0, _e = f, o.styles.forEach(Or, i), _e = null, o.stylesheets.forEach(xn), ya && (f.stylesToHoist = !0), Ht;
  }
  function Jr(i) {
    for (var o = 0; o < i.length; o++) this.push(i[o]);
    i.length = 0;
  }
  var je = [];
  function Vu(i) {
    et(je, i.props);
    for (var o = 0; o < je.length; o++)
      this.push(je[o]);
    je.length = 0, i.state = 2;
  }
  function Ye(i) {
    var o = 0 < i.sheets.size;
    i.sheets.forEach(Vu, this), i.sheets.clear();
    var f = i.rules, g = i.hrefs;
    if (!o || g.length) {
      if (this.push(_e.startInlineStyle), this.push(' data-precedence="'), this.push(i.precedence), i = 0, g.length) {
        for (this.push('" data-href="'); i < g.length - 1; i++)
          this.push(g[i]), this.push(" ");
        this.push(g[i]);
      }
      for (this.push('">'), i = 0; i < f.length; i++)
        this.push(f[i]);
      this.push("</style>"), f.length = 0, g.length = 0;
    }
  }
  function Uo(i) {
    if (i.state === 0) {
      i.state = 1;
      var o = i.props;
      for (et(je, {
        rel: "preload",
        as: "style",
        href: i.props.href,
        crossOrigin: o.crossOrigin,
        fetchPriority: o.fetchPriority,
        integrity: o.integrity,
        media: o.media,
        hrefLang: o.hrefLang,
        referrerPolicy: o.referrerPolicy
      }), i = 0; i < je.length; i++)
        this.push(je[i]);
      je.length = 0;
    }
  }
  function Yo(i) {
    i.sheets.forEach(Uo, this), i.sheets.clear();
  }
  function Go(i, o) {
    !(o.instructions & 32) && (o.instructions |= 32, i.push(
      ' id="',
      ue("_" + o.idPrefix + "R_"),
      '"'
    ));
  }
  function Ul(i, o) {
    i.push("[");
    var f = "[";
    o.stylesheets.forEach(function(g) {
      if (g.state !== 2)
        if (g.state === 3)
          i.push(f), g = Wi(
            "" + g.props.href
          ), i.push(g), i.push("]"), f = ",[";
        else {
          i.push(f);
          var T = g.props["data-precedence"], R = g.props, S = F("" + g.props.href);
          S = Wi(S), i.push(S), T = "" + T, i.push(","), T = Wi(T), i.push(T);
          for (var Y in R)
            if (dn.call(R, Y) && (T = R[Y], T != null))
              switch (Y) {
                case "href":
                case "rel":
                case "precedence":
                case "data-precedence":
                  break;
                case "children":
                case "dangerouslySetInnerHTML":
                  throw Error(G(399, "link"));
                default:
                  hn(
                    i,
                    Y,
                    T
                  );
              }
          i.push("]"), f = ",[", g.state = 3;
        }
    }), i.push("]");
  }
  function hn(i, o, f) {
    var g = o.toLowerCase();
    switch (typeof f) {
      case "function":
      case "symbol":
        return;
    }
    switch (o) {
      case "innerHTML":
      case "dangerouslySetInnerHTML":
      case "suppressContentEditableWarning":
      case "suppressHydrationWarning":
      case "style":
      case "ref":
        return;
      case "className":
        g = "class", o = "" + f;
        break;
      case "hidden":
        if (f === !1) return;
        o = "";
        break;
      case "src":
      case "href":
        f = F(f), o = "" + f;
        break;
      default:
        if (2 < o.length && (o[0] === "o" || o[0] === "O") && (o[1] === "n" || o[1] === "N") || !Li(o))
          return;
        o = "" + f;
    }
    i.push(","), g = Wi(g), i.push(g), i.push(","), g = Wi(o), i.push(g);
  }
  function $e() {
    return { styles: /* @__PURE__ */ new Set(), stylesheets: /* @__PURE__ */ new Set(), suspenseyImages: !1 };
  }
  function Cl(i) {
    var o = ot || null;
    if (o) {
      var f = o.resumableState, g = o.renderState;
      if (typeof i == "string" && i) {
        if (!f.dnsResources.hasOwnProperty(i)) {
          f.dnsResources[i] = null, f = g.headers;
          var T, R;
          (R = f && 0 < f.remainingCapacity) && (R = (T = "<" + ("" + i).replace(
            Ic,
            to
          ) + ">; rel=dns-prefetch", 0 <= (f.remainingCapacity -= T.length + 2))), R ? (g.resets.dns[i] = null, f.preconnects && (f.preconnects += ", "), f.preconnects += T) : (T = [], et(T, { href: i, rel: "dns-prefetch" }), g.preconnects.add(T));
        }
        vo(o);
      }
    } else He.D(i);
  }
  function Ut(i, o) {
    var f = ot || null;
    if (f) {
      var g = f.resumableState, T = f.renderState;
      if (typeof i == "string" && i) {
        var R = o === "use-credentials" ? "credentials" : typeof o == "string" ? "anonymous" : "default";
        if (!g.connectResources[R].hasOwnProperty(i)) {
          g.connectResources[R][i] = null, g = T.headers;
          var S, Y;
          if (Y = g && 0 < g.remainingCapacity) {
            if (Y = "<" + ("" + i).replace(
              Ic,
              to
            ) + ">; rel=preconnect", typeof o == "string") {
              var I = ("" + o).replace(
                Vr,
                Ta
              );
              Y += '; crossorigin="' + I + '"';
            }
            Y = (S = Y, 0 <= (g.remainingCapacity -= S.length + 2));
          }
          Y ? (T.resets.connect[R][i] = null, g.preconnects && (g.preconnects += ", "), g.preconnects += S) : (R = [], et(R, {
            rel: "preconnect",
            href: i,
            crossOrigin: o
          }), T.preconnects.add(R));
        }
        vo(f);
      }
    } else He.C(i, o);
  }
  function xa(i, o, f) {
    var g = ot || null;
    if (g) {
      var T = g.resumableState, R = g.renderState;
      if (o && i) {
        switch (o) {
          case "image":
            if (f)
              var S = f.imageSrcSet, Y = f.imageSizes, I = f.fetchPriority;
            var W = S ? S + `
` + (Y || "") : i;
            if (T.imageResources.hasOwnProperty(W)) return;
            T.imageResources[W] = Ct, T = R.headers;
            var q;
            T && 0 < T.remainingCapacity && typeof S != "string" && I === "high" && (q = In(i, o, f), 0 <= (T.remainingCapacity -= q.length + 2)) ? (R.resets.image[W] = Ct, T.highImagePreloads && (T.highImagePreloads += ", "), T.highImagePreloads += q) : (T = [], et(
              T,
              Rt(
                { rel: "preload", href: S ? void 0 : i, as: o },
                f
              )
            ), I === "high" ? R.highImagePreloads.add(T) : (R.bulkPreloads.add(T), R.preloads.images.set(W, T)));
            break;
          case "style":
            if (T.styleResources.hasOwnProperty(i)) return;
            S = [], et(
              S,
              Rt({ rel: "preload", href: i, as: o }, f)
            ), T.styleResources[i] = !f || typeof f.crossOrigin != "string" && typeof f.integrity != "string" ? Ct : [f.crossOrigin, f.integrity], R.preloads.stylesheets.set(i, S), R.bulkPreloads.add(S);
            break;
          case "script":
            if (T.scriptResources.hasOwnProperty(i)) return;
            S = [], R.preloads.scripts.set(i, S), R.bulkPreloads.add(S), et(
              S,
              Rt({ rel: "preload", href: i, as: o }, f)
            ), T.scriptResources[i] = !f || typeof f.crossOrigin != "string" && typeof f.integrity != "string" ? Ct : [f.crossOrigin, f.integrity];
            break;
          default:
            if (T.unknownResources.hasOwnProperty(o)) {
              if (S = T.unknownResources[o], S.hasOwnProperty(i))
                return;
            } else
              S = {}, T.unknownResources[o] = S;
            if (S[i] = Ct, (T = R.headers) && 0 < T.remainingCapacity && o === "font" && (W = In(i, o, f), 0 <= (T.remainingCapacity -= W.length + 2)))
              R.resets.font[i] = Ct, T.fontPreloads && (T.fontPreloads += ", "), T.fontPreloads += W;
            else
              switch (T = [], i = Rt({ rel: "preload", href: i, as: o }, f), et(T, i), o) {
                case "font":
                  R.fontPreloads.add(T);
                  break;
                default:
                  R.bulkPreloads.add(T);
              }
        }
        vo(g);
      }
    } else He.L(i, o, f);
  }
  function mc(i, o) {
    var f = ot || null;
    if (f) {
      var g = f.resumableState, T = f.renderState;
      if (i) {
        var R = o && typeof o.as == "string" ? o.as : "script";
        switch (R) {
          case "script":
            if (g.moduleScriptResources.hasOwnProperty(i)) return;
            R = [], g.moduleScriptResources[i] = !o || typeof o.crossOrigin != "string" && typeof o.integrity != "string" ? Ct : [o.crossOrigin, o.integrity], T.preloads.moduleScripts.set(i, R);
            break;
          default:
            if (g.moduleUnknownResources.hasOwnProperty(R)) {
              var S = g.unknownResources[R];
              if (S.hasOwnProperty(i)) return;
            } else
              S = {}, g.moduleUnknownResources[R] = S;
            R = [], S[i] = Ct;
        }
        et(R, Rt({ rel: "modulepreload", href: i }, o)), T.bulkPreloads.add(R), vo(f);
      }
    } else He.m(i, o);
  }
  function er(i, o, f) {
    var g = ot || null;
    if (g) {
      var T = g.resumableState, R = g.renderState;
      if (i) {
        o = o || "default";
        var S = R.styles.get(o), Y = T.styleResources.hasOwnProperty(i) ? T.styleResources[i] : void 0;
        Y !== null && (T.styleResources[i] = null, S || (S = {
          precedence: ue(o),
          rules: [],
          hrefs: [],
          sheets: /* @__PURE__ */ new Map()
        }, R.styles.set(o, S)), o = {
          state: 0,
          props: Rt(
            { rel: "stylesheet", href: i, "data-precedence": o },
            f
          )
        }, Y && (Y.length === 2 && Ac(o.props, Y), (R = R.preloads.stylesheets.get(i)) && 0 < R.length ? R.length = 0 : o.state = 1), S.sheets.set(i, o), vo(g));
      }
    } else He.S(i, o, f);
  }
  function Qu(i, o) {
    var f = ot || null;
    if (f) {
      var g = f.resumableState, T = f.renderState;
      if (i) {
        var R = g.scriptResources.hasOwnProperty(i) ? g.scriptResources[i] : void 0;
        R !== null && (g.scriptResources[i] = null, o = Rt({ src: i, async: !0 }, o), R && (R.length === 2 && Ac(o, R), i = T.preloads.scripts.get(i)) && (i.length = 0), i = [], T.scripts.add(i), No(i, o), vo(f));
      }
    } else He.X(i, o);
  }
  function yu(i, o) {
    var f = ot || null;
    if (f) {
      var g = f.resumableState, T = f.renderState;
      if (i) {
        var R = g.moduleScriptResources.hasOwnProperty(
          i
        ) ? g.moduleScriptResources[i] : void 0;
        R !== null && (g.moduleScriptResources[i] = null, o = Rt({ src: i, type: "module", async: !0 }, o), R && (R.length === 2 && Ac(o, R), i = T.preloads.moduleScripts.get(i)) && (i.length = 0), i = [], T.scripts.add(i), No(i, o), vo(f));
      }
    } else He.M(i, o);
  }
  function Ac(i, o) {
    i.crossOrigin == null && (i.crossOrigin = o[0]), i.integrity == null && (i.integrity = o[1]);
  }
  function In(i, o, f) {
    i = ("" + i).replace(
      Ic,
      to
    ), o = ("" + o).replace(
      Vr,
      Ta
    ), o = "<" + i + '>; rel=preload; as="' + o + '"';
    for (var g in f)
      dn.call(f, g) && (i = f[g], typeof i == "string" && (o += "; " + g.toLowerCase() + '="' + ("" + i).replace(
        Vr,
        Ta
      ) + '"'));
    return o;
  }
  var Ic = /[<>\r\n]/g;
  function to(i) {
    switch (i) {
      case "<":
        return "%3C";
      case ">":
        return "%3E";
      case `
`:
        return "%0A";
      case "\r":
        return "%0D";
      default:
        throw Error(
          "escapeLinkHrefForHeaderContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
        );
    }
  }
  var Vr = /["';,\r\n]/g;
  function Ta(i) {
    switch (i) {
      case '"':
        return "%22";
      case "'":
        return "%27";
      case ";":
        return "%3B";
      case ",":
        return "%2C";
      case `
`:
        return "%0A";
      case "\r":
        return "%0D";
      default:
        throw Error(
          "escapeStringForLinkHeaderQuotedParamValueContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
        );
    }
  }
  function Ku(i) {
    this.styles.add(i);
  }
  function qu(i) {
    this.stylesheets.add(i);
  }
  function ro(i, o) {
    o.styles.forEach(Ku, i), o.stylesheets.forEach(qu, i), o.suspenseyImages && (i.suspenseyImages = !0);
  }
  function lo(i, o) {
    var f = i.idPrefix, g = [], T = i.bootstrapScriptContent, R = i.bootstrapScripts, S = i.bootstrapModules;
    T !== void 0 && (g.push("<script"), Go(g, i), g.push(
      ">",
      ("" + T).replace(pl, Mr),
      "<\/script>"
    )), T = f + "P:";
    var Y = f + "S:";
    f += "B:";
    var I = /* @__PURE__ */ new Set(), W = /* @__PURE__ */ new Set(), q = /* @__PURE__ */ new Set(), Q = /* @__PURE__ */ new Map(), ae = /* @__PURE__ */ new Set(), ve = /* @__PURE__ */ new Set(), De = /* @__PURE__ */ new Set(), ke = {
      images: /* @__PURE__ */ new Map(),
      stylesheets: /* @__PURE__ */ new Map(),
      scripts: /* @__PURE__ */ new Map(),
      moduleScripts: /* @__PURE__ */ new Map()
    };
    if (R !== void 0)
      for (var xe = 0; xe < R.length; xe++) {
        var me = R[xe], Ce, _n = void 0, Le = void 0, Je = {
          rel: "preload",
          as: "script",
          fetchPriority: "low",
          nonce: void 0
        };
        typeof me == "string" ? Je.href = Ce = me : (Je.href = Ce = me.src, Je.integrity = Le = typeof me.integrity == "string" ? me.integrity : void 0, Je.crossOrigin = _n = typeof me == "string" || me.crossOrigin == null ? void 0 : me.crossOrigin === "use-credentials" ? "use-credentials" : ""), me = i;
        var on = Ce;
        me.scriptResources[on] = null, me.moduleScriptResources[on] = null, me = [], et(me, Je), ae.add(me), g.push('<script src="', ue(Ce), '"'), typeof Le == "string" && g.push(
          ' integrity="',
          ue(Le),
          '"'
        ), typeof _n == "string" && g.push(
          ' crossorigin="',
          ue(_n),
          '"'
        ), Go(g, i), g.push(' async=""><\/script>');
      }
    if (S !== void 0)
      for (R = 0; R < S.length; R++)
        Je = S[R], _n = Ce = void 0, Le = {
          rel: "modulepreload",
          fetchPriority: "low",
          nonce: void 0
        }, typeof Je == "string" ? Le.href = xe = Je : (Le.href = xe = Je.src, Le.integrity = _n = typeof Je.integrity == "string" ? Je.integrity : void 0, Le.crossOrigin = Ce = typeof Je == "string" || Je.crossOrigin == null ? void 0 : Je.crossOrigin === "use-credentials" ? "use-credentials" : ""), Je = i, me = xe, Je.scriptResources[me] = null, Je.moduleScriptResources[me] = null, Je = [], et(Je, Le), ae.add(Je), g.push(
          '<script type="module" src="',
          ue(xe),
          '"'
        ), typeof _n == "string" && g.push(
          ' integrity="',
          ue(_n),
          '"'
        ), typeof Ce == "string" && g.push(
          ' crossorigin="',
          ue(Ce),
          '"'
        ), Go(g, i), g.push(' async=""><\/script>');
    return {
      placeholderPrefix: T,
      segmentPrefix: Y,
      boundaryPrefix: f,
      startInlineScript: "<script",
      startInlineStyle: "<style",
      preamble: { htmlChunks: null, headChunks: null, bodyChunks: null },
      externalRuntimeScript: null,
      bootstrapChunks: g,
      importMapChunks: [],
      onHeaders: void 0,
      headers: null,
      resets: {
        font: {},
        dns: {},
        connect: { default: {}, anonymous: {}, credentials: {} },
        image: {},
        style: {}
      },
      charsetChunks: [],
      viewportChunks: [],
      hoistableChunks: [],
      preconnects: I,
      fontPreloads: W,
      highImagePreloads: q,
      styles: Q,
      bootstrapScripts: ae,
      scripts: ve,
      bulkPreloads: De,
      preloads: ke,
      nonce: { script: void 0, style: void 0 },
      stylesToHoist: !1,
      generateStaticMarkup: o
    };
  }
  function Yl(i, o, f, g) {
    return f.generateStaticMarkup ? (i.push(ue(o)), !1) : (o === "" ? i = g : (g && i.push("<!-- -->"), i.push(ue(o)), i = !0), i);
  }
  function Gl(i, o, f, g) {
    o.generateStaticMarkup || f && g && i.push("<!-- -->");
  }
  var Mc = Function.prototype.bind, Xo = Symbol.for("react.client.reference");
  function Zo(i) {
    if (i == null) return null;
    if (typeof i == "function")
      return i.$$typeof === Xo ? null : i.displayName || i.name || null;
    if (typeof i == "string") return i;
    switch (i) {
      case kn:
        return "Fragment";
      case Kn:
        return "Profiler";
      case Qe:
        return "StrictMode";
      case H:
        return "Suspense";
      case pe:
        return "SuspenseList";
      case Zr:
        return "Activity";
    }
    if (typeof i == "object")
      switch (i.$$typeof) {
        case Qn:
          return "Portal";
        case Et:
          return i.displayName || "Context";
        case Nl:
          return (i._context.displayName || "Context") + ".Consumer";
        case jt:
          var o = i.render;
          return i = i.displayName, i || (i = o.displayName || o.name || "", i = i !== "" ? "ForwardRef(" + i + ")" : "ForwardRef"), i;
        case Ar:
          return o = i.displayName || null, o !== null ? o : Zo(i.type) || "Memo";
        case ne:
          o = i._payload, i = i._init;
          try {
            return Zo(i(o));
          } catch {
          }
      }
    return null;
  }
  var xt = {}, io = null;
  function Jo(i, o) {
    if (i !== o) {
      i.context._currentValue2 = i.parentValue, i = i.parent;
      var f = o.parent;
      if (i === null) {
        if (f !== null) throw Error(G(401));
      } else {
        if (f === null) throw Error(G(401));
        Jo(i, f);
      }
      o.context._currentValue2 = o.value;
    }
  }
  function Oc(i) {
    i.context._currentValue2 = i.parentValue, i = i.parent, i !== null && Oc(i);
  }
  function Xl(i) {
    var o = i.parent;
    o !== null && Xl(o), i.context._currentValue2 = i.value;
  }
  function Vo(i, o) {
    if (i.context._currentValue2 = i.parentValue, i = i.parent, i === null) throw Error(G(402));
    i.depth === o.depth ? Jo(i, o) : Vo(i, o);
  }
  function Tr(i, o) {
    var f = o.parent;
    if (f === null) throw Error(G(402));
    i.depth === f.depth ? Jo(i, f) : Tr(i, f), o.context._currentValue2 = o.value;
  }
  function kl(i) {
    var o = io;
    o !== i && (o === null ? Xl(i) : i === null ? Oc(o) : o.depth === i.depth ? Jo(o, i) : o.depth > i.depth ? Vo(o, i) : Tr(o, i), io = i);
  }
  var xu = {
    enqueueSetState: function(i, o) {
      i = i._reactInternals, i.queue !== null && i.queue.push(o);
    },
    enqueueReplaceState: function(i, o) {
      i = i._reactInternals, i.replace = !0, i.queue = [o];
    },
    enqueueForceUpdate: function() {
    }
  }, Qo = { id: 1, overflow: "" };
  function wr(i, o, f) {
    var g = i.id;
    i = i.overflow;
    var T = 32 - Ko(g) - 1;
    g &= ~(1 << T), f += 1;
    var R = 32 - Ko(o) + T;
    if (30 < R) {
      var S = T - T % 5;
      return R = (g & (1 << S) - 1).toString(32), g >>= S, T -= S, {
        id: 1 << 32 - Ko(o) + T | f << T | g,
        overflow: R + i
      };
    }
    return {
      id: 1 << R | f << T | g,
      overflow: i
    };
  }
  var Ko = Math.clz32 ? Math.clz32 : pr, Hi = Math.log, ao = Math.LN2;
  function pr(i) {
    return i >>>= 0, i === 0 ? 32 : 31 - (Hi(i) / ao | 0) | 0;
  }
  function St() {
  }
  var Sn = Error(G(460));
  function Tu(i, o, f) {
    switch (f = i[f], f === void 0 ? i.push(o) : f !== o && (o.then(St, St), o = f), o.status) {
      case "fulfilled":
        return o.value;
      case "rejected":
        throw o.reason;
      default:
        switch (typeof o.status == "string" ? o.then(St, St) : (i = o, i.status = "pending", i.then(
          function(g) {
            if (o.status === "pending") {
              var T = o;
              T.status = "fulfilled", T.value = g;
            }
          },
          function(g) {
            if (o.status === "pending") {
              var T = o;
              T.status = "rejected", T.reason = g;
            }
          }
        )), o.status) {
          case "fulfilled":
            return o.value;
          case "rejected":
            throw o.reason;
        }
        throw fi = o, Sn;
    }
  }
  var fi = null;
  function Sl() {
    if (fi === null) throw Error(G(459));
    var i = fi;
    return fi = null, i;
  }
  function wu(i, o) {
    return i === o && (i !== 0 || 1 / i === 1 / o) || i !== i && o !== o;
  }
  var di = typeof Object.is == "function" ? Object.is : wu, Zl = null, oo = null, hi = null, gi = null, co = null, Ie = null, Pl = !1, it = !1, Jl = 0, Tt = 0, Ui = -1, wa = 0, Vl = null, vi = null, at = 0;
  function bi() {
    if (Zl === null)
      throw Error(G(321));
    return Zl;
  }
  function yi() {
    if (0 < at) throw Error(G(312));
    return { memoizedState: null, queue: null, next: null };
  }
  function pa() {
    return Ie === null ? co === null ? (Pl = !1, co = Ie = yi()) : (Pl = !0, Ie = co) : Ie.next === null ? (Pl = !1, Ie = Ie.next = yi()) : (Pl = !0, Ie = Ie.next), Ie;
  }
  function Yi() {
    var i = Vl;
    return Vl = null, i;
  }
  function Ea() {
    gi = hi = oo = Zl = null, it = !1, co = null, at = 0, Ie = vi = null;
  }
  function Er(i, o) {
    return typeof o == "function" ? o(i) : o;
  }
  function Ra(i, o, f) {
    if (Zl = bi(), Ie = pa(), Pl) {
      var g = Ie.queue;
      if (o = g.dispatch, vi !== null && (f = vi.get(g), f !== void 0)) {
        vi.delete(g), g = Ie.memoizedState;
        do
          g = i(g, f.action), f = f.next;
        while (f !== null);
        return Ie.memoizedState = g, [g, o];
      }
      return [Ie.memoizedState, o];
    }
    return i = i === Er ? typeof o == "function" ? o() : o : f !== void 0 ? f(o) : o, Ie.memoizedState = i, i = Ie.queue = { last: null, dispatch: null }, i = i.dispatch = pu.bind(
      null,
      Zl,
      i
    ), [Ie.memoizedState, i];
  }
  function Fl(i, o) {
    if (Zl = bi(), Ie = pa(), o = o === void 0 ? null : o, Ie !== null) {
      var f = Ie.memoizedState;
      if (f !== null && o !== null) {
        var g = f[1];
        e: if (g === null) g = !1;
        else {
          for (var T = 0; T < g.length && T < o.length; T++)
            if (!di(o[T], g[T])) {
              g = !1;
              break e;
            }
          g = !0;
        }
        if (g) return f[0];
      }
    }
    return i = i(), Ie.memoizedState = [i, o], i;
  }
  function pu(i, o, f) {
    if (25 <= at) throw Error(G(301));
    if (i === Zl)
      if (it = !0, i = { action: f, next: null }, vi === null && (vi = /* @__PURE__ */ new Map()), f = vi.get(o), f === void 0)
        vi.set(o, i);
      else {
        for (o = f; o.next !== null; ) o = o.next;
        o.next = i;
      }
  }
  function Eu() {
    throw Error(G(440));
  }
  function Ru() {
    throw Error(G(394));
  }
  function uo() {
    throw Error(G(479));
  }
  function so(i, o, f) {
    bi();
    var g = Tt++, T = hi;
    if (typeof i.$$FORM_ACTION == "function") {
      var R = null, S = gi;
      T = T.formState;
      var Y = i.$$IS_SIGNATURE_EQUAL;
      if (T !== null && typeof Y == "function") {
        var I = T[1];
        Y.call(i, T[2], T[3]) && (R = f !== void 0 ? "p" + f : "k" + Ir(
          JSON.stringify([S, null, g]),
          0
        ), I === R && (Ui = g, o = T[0]));
      }
      var W = i.bind(null, o);
      return i = function(Q) {
        W(Q);
      }, typeof W.$$FORM_ACTION == "function" && (i.$$FORM_ACTION = function(Q) {
        Q = W.$$FORM_ACTION(Q), f !== void 0 && (f += "", Q.action = f);
        var ae = Q.data;
        return ae && (R === null && (R = f !== void 0 ? "p" + f : "k" + Ir(
          JSON.stringify([
            S,
            null,
            g
          ]),
          0
        )), ae.append("$ACTION_KEY", R)), Q;
      }), [o, i, !1];
    }
    var q = i.bind(null, o);
    return [
      o,
      function(Q) {
        q(Q);
      },
      !1
    ];
  }
  function fo(i) {
    var o = wa;
    return wa += 1, Vl === null && (Vl = []), Tu(Vl, i, o);
  }
  function Pt() {
    throw Error(G(393));
  }
  var ju = {
    readContext: function(i) {
      return i._currentValue2;
    },
    use: function(i) {
      if (i !== null && typeof i == "object") {
        if (typeof i.then == "function") return fo(i);
        if (i.$$typeof === Et)
          return i._currentValue2;
      }
      throw Error(G(438, String(i)));
    },
    useContext: function(i) {
      return bi(), i._currentValue2;
    },
    useMemo: Fl,
    useReducer: Ra,
    useRef: function(i) {
      Zl = bi(), Ie = pa();
      var o = Ie.memoizedState;
      return o === null ? (i = { current: i }, Ie.memoizedState = i) : o;
    },
    useState: function(i) {
      return Ra(Er, i);
    },
    useInsertionEffect: St,
    useLayoutEffect: St,
    useCallback: function(i, o) {
      return Fl(function() {
        return i;
      }, o);
    },
    useImperativeHandle: St,
    useEffect: St,
    useDebugValue: St,
    useDeferredValue: function(i, o) {
      return bi(), o !== void 0 ? o : i;
    },
    useTransition: function() {
      return bi(), [!1, Ru];
    },
    useId: function() {
      var i = oo.treeContext, o = i.overflow;
      i = i.id, i = (i & ~(1 << 32 - Ko(i) - 1)).toString(32) + o;
      var f = Gi;
      if (f === null) throw Error(G(404));
      return o = Jl++, i = "_" + f.idPrefix + "R_" + i, 0 < o && (i += "H" + o.toString(32)), i + "_";
    },
    useSyncExternalStore: function(i, o, f) {
      if (f === void 0)
        throw Error(G(407));
      return f();
    },
    useOptimistic: function(i) {
      return bi(), [i, uo];
    },
    useActionState: so,
    useFormState: so,
    useHostTransitionStatus: function() {
      return bi(), Ze;
    },
    useMemoCache: function(i) {
      for (var o = Array(i), f = 0; f < i; f++)
        o[f] = _i;
      return o;
    },
    useCacheRefresh: function() {
      return Pt;
    },
    useEffectEvent: function() {
      return Eu;
    }
  }, Gi = null, qo = {
    getCacheForType: function() {
      throw Error(G(248));
    },
    cacheSignal: function() {
      throw Error(G(248));
    }
  }, _c, xi;
  function Xi(i) {
    if (_c === void 0)
      try {
        throw Error();
      } catch (f) {
        var o = f.stack.trim().match(/\n( *(at )?)/);
        _c = o && o[1] || "", xi = -1 < f.stack.indexOf(`
    at`) ? " (<anonymous>)" : -1 < f.stack.indexOf("@") ? "@unknown:0:0" : "";
      }
    return `
` + _c + i + xi;
  }
  var ho = !1;
  function ml(i, o) {
    if (!i || ho) return "";
    ho = !0;
    var f = Error.prepareStackTrace;
    Error.prepareStackTrace = void 0;
    try {
      var g = {
        DetermineComponentFrameRoot: function() {
          try {
            if (o) {
              var Q = function() {
                throw Error();
              };
              if (Object.defineProperty(Q.prototype, "props", {
                set: function() {
                  throw Error();
                }
              }), typeof Reflect == "object" && Reflect.construct) {
                try {
                  Reflect.construct(Q, []);
                } catch (ve) {
                  var ae = ve;
                }
                Reflect.construct(i, [], Q);
              } else {
                try {
                  Q.call();
                } catch (ve) {
                  ae = ve;
                }
                i.call(Q.prototype);
              }
            } else {
              try {
                throw Error();
              } catch (ve) {
                ae = ve;
              }
              (Q = i()) && typeof Q.catch == "function" && Q.catch(function() {
              });
            }
          } catch (ve) {
            if (ve && ae && typeof ve.stack == "string")
              return [ve.stack, ae.stack];
          }
          return [null, null];
        }
      };
      g.DetermineComponentFrameRoot.displayName = "DetermineComponentFrameRoot";
      var T = Object.getOwnPropertyDescriptor(
        g.DetermineComponentFrameRoot,
        "name"
      );
      T && T.configurable && Object.defineProperty(
        g.DetermineComponentFrameRoot,
        "name",
        { value: "DetermineComponentFrameRoot" }
      );
      var R = g.DetermineComponentFrameRoot(), S = R[0], Y = R[1];
      if (S && Y) {
        var I = S.split(`
`), W = Y.split(`
`);
        for (T = g = 0; g < I.length && !I[g].includes("DetermineComponentFrameRoot"); )
          g++;
        for (; T < W.length && !W[T].includes(
          "DetermineComponentFrameRoot"
        ); )
          T++;
        if (g === I.length || T === W.length)
          for (g = I.length - 1, T = W.length - 1; 1 <= g && 0 <= T && I[g] !== W[T]; )
            T--;
        for (; 1 <= g && 0 <= T; g--, T--)
          if (I[g] !== W[T]) {
            if (g !== 1 || T !== 1)
              do
                if (g--, T--, 0 > T || I[g] !== W[T]) {
                  var q = `
` + I[g].replace(" at new ", " at ");
                  return i.displayName && q.includes("<anonymous>") && (q = q.replace("<anonymous>", i.displayName)), q;
                }
              while (1 <= g && 0 <= T);
            break;
          }
      }
    } finally {
      ho = !1, Error.prepareStackTrace = f;
    }
    return (f = i ? i.displayName || i.name : "") ? Xi(f) : "";
  }
  function Zi(i) {
    if (typeof i == "string") return Xi(i);
    if (typeof i == "function")
      return i.prototype && i.prototype.isReactComponent ? ml(i, !0) : ml(i, !1);
    if (typeof i == "object" && i !== null) {
      switch (i.$$typeof) {
        case jt:
          return ml(i.render, !1);
        case Ar:
          return ml(i.type, !1);
        case ne:
          var o = i, f = o._payload;
          o = o._init;
          try {
            i = o(f);
          } catch {
            return Xi("Lazy");
          }
          return Zi(i);
      }
      if (typeof i.name == "string") {
        e: {
          f = i.name, o = i.env;
          var g = i.debugLocation;
          if (g != null && (i = Error.prepareStackTrace, Error.prepareStackTrace = void 0, g = g.stack, Error.prepareStackTrace = i, g.startsWith(`Error: react-stack-top-frame
`) && (g = g.slice(29)), i = g.indexOf(`
`), i !== -1 && (g = g.slice(i + 1)), i = g.indexOf("react_stack_bottom_frame"), i !== -1 && (i = g.lastIndexOf(`
`, i)), i = i !== -1 ? g = g.slice(0, i) : "", g = i.lastIndexOf(`
`), i = g === -1 ? i : i.slice(g + 1), i.indexOf(f) !== -1)) {
            f = `
` + i;
            break e;
          }
          f = Xi(
            f + (o ? " [" + o + "]" : "")
          );
        }
        return f;
      }
    }
    switch (i) {
      case pe:
        return Xi("SuspenseList");
      case H:
        return Xi("Suspense");
    }
    return "";
  }
  function Ji(i, o) {
    return (500 < o.byteSize || !1) && o.contentPreamble === null;
  }
  function Cu(i) {
    if (typeof i == "object" && i !== null && typeof i.environmentName == "string") {
      var o = i.environmentName;
      i = [i].slice(0), typeof i[0] == "string" ? i.splice(
        0,
        1,
        "[%s] " + i[0],
        " " + o + " "
      ) : i.splice(0, 0, "[%s]", " " + o + " "), i.unshift(console), o = Mc.apply(console.error, i), o();
    } else console.error(i);
    return null;
  }
  function vs(i, o, f, g, T, R, S, Y, I, W, q) {
    var Q = /* @__PURE__ */ new Set();
    this.destination = null, this.flushScheduled = !1, this.resumableState = i, this.renderState = o, this.rootFormatContext = f, this.progressiveChunkSize = g === void 0 ? 12800 : g, this.status = 10, this.fatalError = null, this.pendingRootTasks = this.allPendingTasks = this.nextSegmentId = 0, this.completedPreambleSegments = this.completedRootSegment = null, this.byteSize = 0, this.abortableTasks = Q, this.pingedTasks = [], this.clientRenderedBoundaries = [], this.completedBoundaries = [], this.partialBoundaries = [], this.trackedPostpones = null, this.onError = T === void 0 ? Cu : T, this.onPostpone = W === void 0 ? St : W, this.onAllReady = R === void 0 ? St : R, this.onShellReady = S === void 0 ? St : S, this.onShellError = Y === void 0 ? St : Y, this.onFatalError = I === void 0 ? St : I, this.formState = q === void 0 ? null : q;
  }
  function Ca(i, o, f, g, T, R, S, Y, I, W, q, Q) {
    return o = new vs(
      o,
      f,
      g,
      T,
      R,
      S,
      Y,
      I,
      W,
      q,
      Q
    ), f = Qr(
      o,
      0,
      null,
      g,
      !1,
      !1
    ), f.parentFlushed = !0, i = $o(
      o,
      null,
      i,
      -1,
      null,
      f,
      null,
      null,
      o.abortableTasks,
      null,
      g,
      null,
      Qo,
      null,
      null
    ), Kr(i), o.pingedTasks.push(i), o;
  }
  var ot = null;
  function Ql(i, o) {
    i.pingedTasks.push(o), i.pingedTasks.length === 1 && (i.flushScheduled = i.destination !== null, es(i));
  }
  function jo(i, o, f, g, T) {
    return f = {
      status: 0,
      rootSegmentID: -1,
      parentFlushed: !1,
      pendingTasks: 0,
      row: o,
      completedSegments: [],
      byteSize: 0,
      fallbackAbortableTasks: f,
      errorDigest: null,
      contentState: $e(),
      fallbackState: $e(),
      contentPreamble: g,
      fallbackPreamble: T,
      trackedContentKeyPath: null,
      trackedFallbackNode: null
    }, o !== null && (o.pendingTasks++, g = o.boundaries, g !== null && (i.allPendingTasks++, f.pendingTasks++, g.push(f)), i = o.inheritedHoistables, i !== null && ro(f.contentState, i)), f;
  }
  function $o(i, o, f, g, T, R, S, Y, I, W, q, Q, ae, ve, De) {
    i.allPendingTasks++, T === null ? i.pendingRootTasks++ : T.pendingTasks++, ve !== null && ve.pendingTasks++;
    var ke = {
      replay: null,
      node: f,
      childIndex: g,
      ping: function() {
        return Ql(i, ke);
      },
      blockedBoundary: T,
      blockedSegment: R,
      blockedPreamble: S,
      hoistableState: Y,
      abortSet: I,
      keyPath: W,
      formatContext: q,
      context: Q,
      treeContext: ae,
      row: ve,
      componentStack: De,
      thenableState: o
    };
    return I.add(ke), ke;
  }
  function Dc(i, o, f, g, T, R, S, Y, I, W, q, Q, ae, ve) {
    i.allPendingTasks++, R === null ? i.pendingRootTasks++ : R.pendingTasks++, ae !== null && ae.pendingTasks++, f.pendingTasks++;
    var De = {
      replay: f,
      node: g,
      childIndex: T,
      ping: function() {
        return Ql(i, De);
      },
      blockedBoundary: R,
      blockedSegment: null,
      blockedPreamble: null,
      hoistableState: S,
      abortSet: Y,
      keyPath: I,
      formatContext: W,
      context: q,
      treeContext: Q,
      row: ae,
      componentStack: ve,
      thenableState: o
    };
    return Y.add(De), De;
  }
  function Qr(i, o, f, g, T, R) {
    return {
      status: 0,
      parentFlushed: !1,
      id: -1,
      index: o,
      chunks: [],
      children: [],
      preambleChildren: [],
      parentFormatContext: g,
      boundary: f,
      lastPushedText: T,
      textEmbedded: R
    };
  }
  function Kr(i) {
    var o = i.node;
    if (typeof o == "object" && o !== null)
      switch (o.$$typeof) {
        case vt:
          i.componentStack = { parent: i.componentStack, type: o.type };
      }
  }
  function _r(i) {
    return i === null ? null : { parent: i.parent, type: "Suspense Fallback" };
  }
  function qr(i) {
    var o = {};
    return i && Object.defineProperty(o, "componentStack", {
      configurable: !0,
      enumerable: !0,
      get: function() {
        try {
          var f = "", g = i;
          do
            f += Zi(g.type), g = g.parent;
          while (g);
          var T = f;
        } catch (R) {
          T = `
Error generating stack: ` + R.message + `
` + R.stack;
        }
        return Object.defineProperty(o, "componentStack", {
          value: T
        }), T;
      }
    }), o;
  }
  function Tn(i, o, f) {
    if (i = i.onError, o = i(o, f), o == null || typeof o == "string") return o;
  }
  function jr(i, o) {
    var f = i.onShellError, g = i.onFatalError;
    f(o), g(o), i.destination !== null ? (i.status = 14, i.destination.destroy(o)) : (i.status = 13, i.fatalError = o);
  }
  function Mn(i, o) {
    Al(i, o.next, o.hoistables);
  }
  function Al(i, o, f) {
    for (; o !== null; ) {
      f !== null && (ro(o.hoistables, f), o.inheritedHoistables = f);
      var g = o.boundaries;
      if (g !== null) {
        o.boundaries = null;
        for (var T = 0; T < g.length; T++) {
          var R = g[T];
          f !== null && ro(R.contentState, f), wi(i, R, null, null);
        }
      }
      if (o.pendingTasks--, 0 < o.pendingTasks) break;
      f = o.hoistables, o = o.next;
    }
  }
  function Lc(i, o) {
    var f = o.boundaries;
    if (f !== null && o.pendingTasks === f.length) {
      for (var g = !0, T = 0; T < f.length; T++) {
        var R = f[T];
        if (R.pendingTasks !== 1 || R.parentFlushed || Ji(i, R)) {
          g = !1;
          break;
        }
      }
      g && Al(i, o, o.hoistables);
    }
  }
  function zc(i) {
    var o = {
      pendingTasks: 1,
      boundaries: null,
      hoistables: $e(),
      inheritedHoistables: null,
      together: !1,
      next: null
    };
    return i !== null && 0 < i.pendingTasks && (o.pendingTasks++, o.boundaries = [], i.next = o), o;
  }
  function Bc(i, o, f, g, T) {
    var R = o.keyPath, S = o.treeContext, Y = o.row;
    o.keyPath = f, f = g.length;
    var I = null;
    if (o.replay !== null) {
      var W = o.replay.slots;
      if (W !== null && typeof W == "object")
        for (var q = 0; q < f; q++) {
          var Q = T !== "backwards" && T !== "unstable_legacy-backwards" ? q : f - 1 - q, ae = g[Q];
          o.row = I = zc(
            I
          ), o.treeContext = wr(S, f, Q);
          var ve = W[Q];
          typeof ve == "number" ? (tc(i, o, ve, ae, Q), delete W[Q]) : _t(i, o, ae, Q), --I.pendingTasks === 0 && Mn(i, I);
        }
      else
        for (W = 0; W < f; W++)
          q = T !== "backwards" && T !== "unstable_legacy-backwards" ? W : f - 1 - W, Q = g[q], o.row = I = zc(I), o.treeContext = wr(S, f, q), _t(i, o, Q, q), --I.pendingTasks === 0 && Mn(i, I);
    } else if (T !== "backwards" && T !== "unstable_legacy-backwards")
      for (T = 0; T < f; T++)
        W = g[T], o.row = I = zc(I), o.treeContext = wr(
          S,
          f,
          T
        ), _t(i, o, W, T), --I.pendingTasks === 0 && Mn(i, I);
    else {
      for (T = o.blockedSegment, W = T.children.length, q = T.chunks.length, Q = f - 1; 0 <= Q; Q--) {
        ae = g[Q], o.row = I = zc(
          I
        ), o.treeContext = wr(S, f, Q), ve = Qr(
          i,
          q,
          null,
          o.formatContext,
          Q === 0 ? T.lastPushedText : !0,
          !0
        ), T.children.splice(W, 0, ve), o.blockedSegment = ve;
        try {
          _t(i, o, ae, Q), Gl(
            ve.chunks,
            i.renderState,
            ve.lastPushedText,
            ve.textEmbedded
          ), ve.status = 1, --I.pendingTasks === 0 && Mn(i, I);
        } catch (De) {
          throw ve.status = i.status === 12 ? 3 : 4, De;
        }
      }
      o.blockedSegment = T, T.lastPushedText = !1;
    }
    Y !== null && I !== null && 0 < I.pendingTasks && (Y.pendingTasks++, I.next = Y), o.treeContext = S, o.row = Y, o.keyPath = R;
  }
  function ec(i, o, f, g, T, R) {
    var S = o.thenableState;
    for (o.thenableState = null, Zl = {}, oo = o, hi = i, gi = f, Tt = Jl = 0, Ui = -1, wa = 0, Vl = S, i = g(T, R); it; )
      it = !1, Tt = Jl = 0, Ui = -1, wa = 0, at += 1, Ie = null, i = g(T, R);
    return Ea(), i;
  }
  function Ti(i, o, f, g, T, R, S) {
    var Y = !1;
    if (R !== 0 && i.formState !== null) {
      var I = o.blockedSegment;
      if (I !== null) {
        Y = !0, I = I.chunks;
        for (var W = 0; W < R; W++)
          W === S ? I.push("<!--F!-->") : I.push("<!--F-->");
      }
    }
    R = o.keyPath, o.keyPath = f, T ? (f = o.treeContext, o.treeContext = wr(f, 1, 0), _t(i, o, g, -1), o.treeContext = f) : Y ? _t(i, o, g, -1) : Rr(i, o, g, -1), o.keyPath = R;
  }
  function nc(i, o, f, g, T, R) {
    if (typeof g == "function")
      if (g.prototype && g.prototype.isReactComponent) {
        var S = T;
        if ("ref" in T) {
          S = {};
          for (var Y in T)
            Y !== "ref" && (S[Y] = T[Y]);
        }
        var I = g.defaultProps;
        if (I) {
          S === T && (S = Rt({}, S, T));
          for (var W in I)
            S[W] === void 0 && (S[W] = I[W]);
        }
        T = S, S = xt, I = g.contextType, typeof I == "object" && I !== null && (S = I._currentValue2), S = new g(T, S);
        var q = S.state !== void 0 ? S.state : null;
        if (S.updater = xu, S.props = T, S.state = q, I = { queue: [], replace: !1 }, S._reactInternals = I, R = g.contextType, S.context = typeof R == "object" && R !== null ? R._currentValue2 : xt, R = g.getDerivedStateFromProps, typeof R == "function" && (R = R(T, q), q = R == null ? q : Rt({}, q, R), S.state = q), typeof g.getDerivedStateFromProps != "function" && typeof S.getSnapshotBeforeUpdate != "function" && (typeof S.UNSAFE_componentWillMount == "function" || typeof S.componentWillMount == "function"))
          if (g = S.state, typeof S.componentWillMount == "function" && S.componentWillMount(), typeof S.UNSAFE_componentWillMount == "function" && S.UNSAFE_componentWillMount(), g !== S.state && xu.enqueueReplaceState(
            S,
            S.state,
            null
          ), I.queue !== null && 0 < I.queue.length)
            if (g = I.queue, R = I.replace, I.queue = null, I.replace = !1, R && g.length === 1)
              S.state = g[0];
            else {
              for (I = R ? g[0] : S.state, q = !0, R = R ? 1 : 0; R < g.length; R++)
                W = g[R], W = typeof W == "function" ? W.call(S, I, T, void 0) : W, W != null && (q ? (q = !1, I = Rt({}, I, W)) : Rt(I, W));
              S.state = I;
            }
          else I.queue = null;
        if (g = S.render(), i.status === 12) throw null;
        T = o.keyPath, o.keyPath = f, Rr(i, o, g, -1), o.keyPath = T;
      } else {
        if (g = ec(i, o, f, g, T, void 0), i.status === 12) throw null;
        Ti(
          i,
          o,
          f,
          g,
          Jl !== 0,
          Tt,
          Ui
        );
      }
    else if (typeof g == "string")
      if (S = o.blockedSegment, S === null)
        S = T.children, I = o.formatContext, q = o.keyPath, o.formatContext = Sc(I, g, T), o.keyPath = f, _t(i, o, S, -1), o.formatContext = I, o.keyPath = q;
      else {
        if (q = An(
          S.chunks,
          g,
          T,
          i.resumableState,
          i.renderState,
          o.blockedPreamble,
          o.hoistableState,
          o.formatContext,
          S.lastPushedText
        ), S.lastPushedText = !1, I = o.formatContext, R = o.keyPath, o.keyPath = f, (o.formatContext = Sc(I, g, T)).insertionMode === 3) {
          f = Qr(
            i,
            0,
            null,
            o.formatContext,
            !1,
            !1
          ), S.preambleChildren.push(f), o.blockedSegment = f;
          try {
            f.status = 6, _t(i, o, q, -1), Gl(
              f.chunks,
              i.renderState,
              f.lastPushedText,
              f.textEmbedded
            ), f.status = 1;
          } finally {
            o.blockedSegment = S;
          }
        } else _t(i, o, q, -1);
        o.formatContext = I, o.keyPath = R;
        e: {
          switch (o = S.chunks, i = i.resumableState, g) {
            case "title":
            case "style":
            case "script":
            case "area":
            case "base":
            case "br":
            case "col":
            case "embed":
            case "hr":
            case "img":
            case "input":
            case "keygen":
            case "link":
            case "meta":
            case "param":
            case "source":
            case "track":
            case "wbr":
              break e;
            case "body":
              if (1 >= I.insertionMode) {
                i.hasBody = !0;
                break e;
              }
              break;
            case "html":
              if (I.insertionMode === 0) {
                i.hasHtml = !0;
                break e;
              }
              break;
            case "head":
              if (1 >= I.insertionMode) break e;
          }
          o.push(va(g));
        }
        S.lastPushedText = !1;
      }
    else {
      switch (g) {
        case zo:
        case Qe:
        case Kn:
        case kn:
          g = o.keyPath, o.keyPath = f, Rr(i, o, T.children, -1), o.keyPath = g;
          return;
        case Zr:
          g = o.blockedSegment, g === null ? T.mode !== "hidden" && (g = o.keyPath, o.keyPath = f, _t(i, o, T.children, -1), o.keyPath = g) : T.mode !== "hidden" && (i.renderState.generateStaticMarkup || g.chunks.push("<!--&-->"), g.lastPushedText = !1, S = o.keyPath, o.keyPath = f, _t(i, o, T.children, -1), o.keyPath = S, i.renderState.generateStaticMarkup || g.chunks.push("<!--/&-->"), g.lastPushedText = !1);
          return;
        case pe:
          e: {
            if (g = T.children, T = T.revealOrder, T === "forwards" || T === "backwards" || T === "unstable_legacy-backwards") {
              if (Ee(g)) {
                Bc(i, o, f, g, T);
                break e;
              }
              if ((S = Nt(g)) && (S = S.call(g))) {
                if (I = S.next(), !I.done) {
                  do
                    I = S.next();
                  while (!I.done);
                  Bc(i, o, f, g, T);
                }
                break e;
              }
            }
            T === "together" ? (T = o.keyPath, S = o.row, I = o.row = zc(null), I.boundaries = [], I.together = !0, o.keyPath = f, Rr(i, o, g, -1), --I.pendingTasks === 0 && Mn(i, I), o.keyPath = T, o.row = S, S !== null && 0 < I.pendingTasks && (S.pendingTasks++, I.next = S)) : (T = o.keyPath, o.keyPath = f, Rr(i, o, g, -1), o.keyPath = T);
          }
          return;
        case Bo:
        case N:
          throw Error(G(343));
        case H:
          e: if (o.replay !== null) {
            g = o.keyPath, S = o.formatContext, I = o.row, o.keyPath = f, o.formatContext = un(
              i.resumableState,
              S
            ), o.row = null, f = T.children;
            try {
              _t(i, o, f, -1);
            } finally {
              o.keyPath = g, o.formatContext = S, o.row = I;
            }
          } else {
            g = o.keyPath, R = o.formatContext;
            var Q = o.row, ae = o.blockedBoundary;
            W = o.blockedPreamble;
            var ve = o.hoistableState;
            Y = o.blockedSegment;
            var De = T.fallback;
            T = T.children;
            var ke = /* @__PURE__ */ new Set(), xe = jo(
              i,
              o.row,
              ke,
              null,
              null
            );
            i.trackedPostpones !== null && (xe.trackedContentKeyPath = f);
            var me = Qr(
              i,
              Y.chunks.length,
              xe,
              o.formatContext,
              !1,
              !1
            );
            Y.children.push(me), Y.lastPushedText = !1;
            var Ce = Qr(
              i,
              0,
              null,
              o.formatContext,
              !1,
              !1
            );
            if (Ce.parentFlushed = !0, i.trackedPostpones !== null) {
              S = o.componentStack, I = [f[0], "Suspense Fallback", f[2]], q = [I[1], I[2], [], null], i.trackedPostpones.workingMap.set(I, q), xe.trackedFallbackNode = q, o.blockedSegment = me, o.blockedPreamble = xe.fallbackPreamble, o.keyPath = I, o.formatContext = lt(
                i.resumableState,
                R
              ), o.componentStack = _r(S), me.status = 6;
              try {
                _t(i, o, De, -1), Gl(
                  me.chunks,
                  i.renderState,
                  me.lastPushedText,
                  me.textEmbedded
                ), me.status = 1;
              } catch (_n) {
                throw me.status = i.status === 12 ? 3 : 4, _n;
              } finally {
                o.blockedSegment = Y, o.blockedPreamble = W, o.keyPath = g, o.formatContext = R;
              }
              o = $o(
                i,
                null,
                T,
                -1,
                xe,
                Ce,
                xe.contentPreamble,
                xe.contentState,
                o.abortSet,
                f,
                un(
                  i.resumableState,
                  o.formatContext
                ),
                o.context,
                o.treeContext,
                null,
                S
              ), Kr(o), i.pingedTasks.push(o);
            } else {
              o.blockedBoundary = xe, o.blockedPreamble = xe.contentPreamble, o.hoistableState = xe.contentState, o.blockedSegment = Ce, o.keyPath = f, o.formatContext = un(
                i.resumableState,
                R
              ), o.row = null, Ce.status = 6;
              try {
                if (_t(i, o, T, -1), Gl(
                  Ce.chunks,
                  i.renderState,
                  Ce.lastPushedText,
                  Ce.textEmbedded
                ), Ce.status = 1, Yt(xe, Ce), xe.pendingTasks === 0 && xe.status === 0) {
                  if (xe.status = 1, !Ji(i, xe)) {
                    Q !== null && --Q.pendingTasks === 0 && Mn(i, Q), i.pendingRootTasks === 0 && o.blockedPreamble && Fa(i);
                    break e;
                  }
                } else
                  Q !== null && Q.together && Lc(i, Q);
              } catch (_n) {
                xe.status = 4, i.status === 12 ? (Ce.status = 3, S = i.fatalError) : (Ce.status = 4, S = _n), I = qr(o.componentStack), q = Tn(
                  i,
                  S,
                  I
                ), xe.errorDigest = q, Sa(i, xe);
              } finally {
                o.blockedBoundary = ae, o.blockedPreamble = W, o.hoistableState = ve, o.blockedSegment = Y, o.keyPath = g, o.formatContext = R, o.row = Q;
              }
              o = $o(
                i,
                null,
                De,
                -1,
                ae,
                me,
                xe.fallbackPreamble,
                xe.fallbackState,
                ke,
                [f[0], "Suspense Fallback", f[2]],
                lt(
                  i.resumableState,
                  o.formatContext
                ),
                o.context,
                o.treeContext,
                o.row,
                _r(
                  o.componentStack
                )
              ), Kr(o), i.pingedTasks.push(o);
            }
          }
          return;
      }
      if (typeof g == "object" && g !== null)
        switch (g.$$typeof) {
          case jt:
            if ("ref" in T)
              for (De in S = {}, T)
                De !== "ref" && (S[De] = T[De]);
            else S = T;
            g = ec(
              i,
              o,
              f,
              g.render,
              S,
              R
            ), Ti(
              i,
              o,
              f,
              g,
              Jl !== 0,
              Tt,
              Ui
            );
            return;
          case Ar:
            nc(i, o, f, g.type, T, R);
            return;
          case Et:
            if (I = T.children, S = o.keyPath, T = T.value, q = g._currentValue2, g._currentValue2 = T, R = io, io = g = {
              parent: R,
              depth: R === null ? 0 : R.depth + 1,
              context: g,
              parentValue: q,
              value: T
            }, o.context = g, o.keyPath = f, Rr(i, o, I, -1), i = io, i === null) throw Error(G(403));
            i.context._currentValue2 = i.parentValue, i = io = i.parent, o.context = i, o.keyPath = S;
            return;
          case Nl:
            T = T.children, g = T(g._context._currentValue2), T = o.keyPath, o.keyPath = f, Rr(i, o, g, -1), o.keyPath = T;
            return;
          case ne:
            if (S = g._init, g = S(g._payload), i.status === 12) throw null;
            nc(i, o, f, g, T, R);
            return;
        }
      throw Error(
        G(130, g == null ? g : typeof g, "")
      );
    }
  }
  function tc(i, o, f, g, T) {
    var R = o.replay, S = o.blockedBoundary, Y = Qr(
      i,
      0,
      null,
      o.formatContext,
      !1,
      !1
    );
    Y.id = f, Y.parentFlushed = !0;
    try {
      o.replay = null, o.blockedSegment = Y, _t(i, o, g, T), Y.status = 1, S === null ? i.completedRootSegment = Y : (Yt(S, Y), S.parentFlushed && i.partialBoundaries.push(S));
    } finally {
      o.replay = R, o.blockedSegment = null;
    }
  }
  function Rr(i, o, f, g) {
    o.replay !== null && typeof o.replay.slots == "number" ? tc(i, o, o.replay.slots, f, g) : (o.node = f, o.childIndex = g, f = o.componentStack, Kr(o), On(i, o), o.componentStack = f);
  }
  function On(i, o) {
    var f = o.node, g = o.childIndex;
    if (f !== null) {
      if (typeof f == "object") {
        switch (f.$$typeof) {
          case vt:
            var T = f.type, R = f.key, S = f.props;
            f = S.ref;
            var Y = f !== void 0 ? f : null, I = Zo(T), W = R ?? (g === -1 ? 0 : g);
            if (R = [o.keyPath, I, W], o.replay !== null)
              e: {
                var q = o.replay;
                for (g = q.nodes, f = 0; f < g.length; f++) {
                  var Q = g[f];
                  if (W === Q[1]) {
                    if (Q.length === 4) {
                      if (I !== null && I !== Q[0])
                        throw Error(
                          G(490, Q[0], I)
                        );
                      var ae = Q[2];
                      I = Q[3], W = o.node, o.replay = {
                        nodes: ae,
                        slots: I,
                        pendingTasks: 1
                      };
                      try {
                        if (nc(i, o, R, T, S, Y), o.replay.pendingTasks === 1 && 0 < o.replay.nodes.length)
                          throw Error(G(488));
                        o.replay.pendingTasks--;
                      } catch (on) {
                        if (typeof on == "object" && on !== null && (on === Sn || typeof on.then == "function"))
                          throw o.node === W ? o.replay = q : g.splice(f, 1), on;
                        o.replay.pendingTasks--, S = qr(o.componentStack), R = i, i = o.blockedBoundary, T = on, S = Tn(R, T, S), Il(
                          R,
                          i,
                          ae,
                          I,
                          T,
                          S
                        );
                      }
                      o.replay = q;
                    } else {
                      if (T !== H)
                        throw Error(
                          G(
                            490,
                            "Suspense",
                            Zo(T) || "Unknown"
                          )
                        );
                      n: {
                        q = void 0, T = Q[5], Y = Q[2], I = Q[3], W = Q[4] === null ? [] : Q[4][2], Q = Q[4] === null ? null : Q[4][3];
                        var ve = o.keyPath, De = o.formatContext, ke = o.row, xe = o.replay, me = o.blockedBoundary, Ce = o.hoistableState, _n = S.children, Le = S.fallback, Je = /* @__PURE__ */ new Set();
                        S = jo(
                          i,
                          o.row,
                          Je,
                          null,
                          null
                        ), S.parentFlushed = !0, S.rootSegmentID = T, o.blockedBoundary = S, o.hoistableState = S.contentState, o.keyPath = R, o.formatContext = un(
                          i.resumableState,
                          De
                        ), o.row = null, o.replay = {
                          nodes: Y,
                          slots: I,
                          pendingTasks: 1
                        };
                        try {
                          if (_t(i, o, _n, -1), o.replay.pendingTasks === 1 && 0 < o.replay.nodes.length)
                            throw Error(G(488));
                          if (o.replay.pendingTasks--, S.pendingTasks === 0 && S.status === 0) {
                            S.status = 1, i.completedBoundaries.push(S);
                            break n;
                          }
                        } catch (on) {
                          S.status = 4, ae = qr(o.componentStack), q = Tn(
                            i,
                            on,
                            ae
                          ), S.errorDigest = q, o.replay.pendingTasks--, i.clientRenderedBoundaries.push(S);
                        } finally {
                          o.blockedBoundary = me, o.hoistableState = Ce, o.replay = xe, o.keyPath = ve, o.formatContext = De, o.row = ke;
                        }
                        ae = Dc(
                          i,
                          null,
                          {
                            nodes: W,
                            slots: Q,
                            pendingTasks: 0
                          },
                          Le,
                          -1,
                          me,
                          S.fallbackState,
                          Je,
                          [R[0], "Suspense Fallback", R[2]],
                          lt(
                            i.resumableState,
                            o.formatContext
                          ),
                          o.context,
                          o.treeContext,
                          o.row,
                          _r(
                            o.componentStack
                          )
                        ), Kr(ae), i.pingedTasks.push(ae);
                      }
                    }
                    g.splice(f, 1);
                    break e;
                  }
                }
              }
            else nc(i, o, R, T, S, Y);
            return;
          case Qn:
            throw Error(G(257));
          case ne:
            if (ae = f._init, f = ae(f._payload), i.status === 12) throw null;
            Rr(i, o, f, g);
            return;
        }
        if (Ee(f)) {
          en(i, o, f, g);
          return;
        }
        if ((ae = Nt(f)) && (ae = ae.call(f))) {
          if (f = ae.next(), !f.done) {
            S = [];
            do
              S.push(f.value), f = ae.next();
            while (!f.done);
            en(i, o, S, g);
          }
          return;
        }
        if (typeof f.then == "function")
          return o.thenableState = null, Rr(i, o, fo(f), g);
        if (f.$$typeof === Et)
          return Rr(
            i,
            o,
            f._currentValue2,
            g
          );
        throw g = Object.prototype.toString.call(f), Error(
          G(
            31,
            g === "[object Object]" ? "object with keys {" + Object.keys(f).join(", ") + "}" : g
          )
        );
      }
      typeof f == "string" ? (g = o.blockedSegment, g !== null && (g.lastPushedText = Yl(
        g.chunks,
        f,
        i.renderState,
        g.lastPushedText
      ))) : (typeof f == "number" || typeof f == "bigint") && (g = o.blockedSegment, g !== null && (g.lastPushedText = Yl(
        g.chunks,
        "" + f,
        i.renderState,
        g.lastPushedText
      )));
    }
  }
  function en(i, o, f, g) {
    var T = o.keyPath;
    if (g !== -1 && (o.keyPath = [o.keyPath, "Fragment", g], o.replay !== null)) {
      for (var R = o.replay, S = R.nodes, Y = 0; Y < S.length; Y++) {
        var I = S[Y];
        if (I[1] === g) {
          g = I[2], I = I[3], o.replay = { nodes: g, slots: I, pendingTasks: 1 };
          try {
            if (en(i, o, f, -1), o.replay.pendingTasks === 1 && 0 < o.replay.nodes.length)
              throw Error(G(488));
            o.replay.pendingTasks--;
          } catch (Q) {
            if (typeof Q == "object" && Q !== null && (Q === Sn || typeof Q.then == "function"))
              throw Q;
            o.replay.pendingTasks--, f = qr(o.componentStack);
            var W = o.blockedBoundary, q = Q;
            f = Tn(i, q, f), Il(
              i,
              W,
              g,
              I,
              q,
              f
            );
          }
          o.replay = R, S.splice(Y, 1);
          break;
        }
      }
      o.keyPath = T;
      return;
    }
    if (R = o.treeContext, S = f.length, o.replay !== null && (Y = o.replay.slots, Y !== null && typeof Y == "object")) {
      for (g = 0; g < S; g++)
        I = f[g], o.treeContext = wr(R, S, g), W = Y[g], typeof W == "number" ? (tc(i, o, W, I, g), delete Y[g]) : _t(i, o, I, g);
      o.treeContext = R, o.keyPath = T;
      return;
    }
    for (Y = 0; Y < S; Y++)
      g = f[Y], o.treeContext = wr(R, S, Y), _t(i, o, g, Y);
    o.treeContext = R, o.keyPath = T;
  }
  function Vi(i, o, f) {
    if (f.status = 5, f.rootSegmentID = i.nextSegmentId++, i = f.trackedContentKeyPath, i === null) throw Error(G(486));
    var g = f.trackedFallbackNode, T = [], R = o.workingMap.get(i);
    return R === void 0 ? (f = [
      i[1],
      i[2],
      T,
      null,
      g,
      f.rootSegmentID
    ], o.workingMap.set(i, f), Wn(f, i[0], o), f) : (R[4] = g, R[5] = f.rootSegmentID, R);
  }
  function ka(i, o, f, g) {
    g.status = 5;
    var T = f.keyPath, R = f.blockedBoundary;
    if (R === null)
      g.id = i.nextSegmentId++, o.rootSlots = g.id, i.completedRootSegment !== null && (i.completedRootSegment.status = 5);
    else {
      if (R !== null && R.status === 0) {
        var S = Vi(
          i,
          o,
          R
        );
        if (R.trackedContentKeyPath === T && f.childIndex === -1) {
          g.id === -1 && (g.id = g.parentFlushed ? R.rootSegmentID : i.nextSegmentId++), S[3] = g.id;
          return;
        }
      }
      if (g.id === -1 && (g.id = g.parentFlushed && R !== null ? R.rootSegmentID : i.nextSegmentId++), f.childIndex === -1)
        T === null ? o.rootSlots = g.id : (f = o.workingMap.get(T), f === void 0 ? (f = [T[1], T[2], [], g.id], Wn(f, T[0], o)) : f[3] = g.id);
      else {
        if (T === null) {
          if (i = o.rootSlots, i === null)
            i = o.rootSlots = {};
          else if (typeof i == "number")
            throw Error(G(491));
        } else if (R = o.workingMap, S = R.get(T), S === void 0)
          i = {}, S = [T[1], T[2], [], i], R.set(T, S), Wn(S, T[0], o);
        else if (i = S[3], i === null)
          i = S[3] = {};
        else if (typeof i == "number")
          throw Error(G(491));
        i[f.childIndex] = g.id;
      }
    }
  }
  function Sa(i, o) {
    i = i.trackedPostpones, i !== null && (o = o.trackedContentKeyPath, o !== null && (o = i.workingMap.get(o), o !== void 0 && (o.length = 4, o[2] = [], o[3] = null)));
  }
  function Qi(i, o, f) {
    return Dc(
      i,
      f,
      o.replay,
      o.node,
      o.childIndex,
      o.blockedBoundary,
      o.hoistableState,
      o.abortSet,
      o.keyPath,
      o.formatContext,
      o.context,
      o.treeContext,
      o.row,
      o.componentStack
    );
  }
  function ku(i, o, f) {
    var g = o.blockedSegment, T = Qr(
      i,
      g.chunks.length,
      null,
      o.formatContext,
      g.lastPushedText,
      !0
    );
    return g.children.push(T), g.lastPushedText = !1, $o(
      i,
      f,
      o.node,
      o.childIndex,
      o.blockedBoundary,
      T,
      o.blockedPreamble,
      o.hoistableState,
      o.abortSet,
      o.keyPath,
      o.formatContext,
      o.context,
      o.treeContext,
      o.row,
      o.componentStack
    );
  }
  function _t(i, o, f, g) {
    var T = o.formatContext, R = o.context, S = o.keyPath, Y = o.treeContext, I = o.componentStack, W = o.blockedSegment;
    if (W === null) {
      W = o.replay;
      try {
        return Rr(i, o, f, g);
      } catch (ae) {
        if (Ea(), f = ae === Sn ? Sl() : ae, i.status !== 12 && typeof f == "object" && f !== null) {
          if (typeof f.then == "function") {
            g = ae === Sn ? Yi() : null, i = Qi(i, o, g).ping, f.then(i, i), o.formatContext = T, o.context = R, o.keyPath = S, o.treeContext = Y, o.componentStack = I, o.replay = W, kl(R);
            return;
          }
          if (f.message === "Maximum call stack size exceeded") {
            f = ae === Sn ? Yi() : null, f = Qi(i, o, f), i.pingedTasks.push(f), o.formatContext = T, o.context = R, o.keyPath = S, o.treeContext = Y, o.componentStack = I, o.replay = W, kl(R);
            return;
          }
        }
      }
    } else {
      var q = W.children.length, Q = W.chunks.length;
      try {
        return Rr(i, o, f, g);
      } catch (ae) {
        if (Ea(), W.children.length = q, W.chunks.length = Q, f = ae === Sn ? Sl() : ae, i.status !== 12 && typeof f == "object" && f !== null) {
          if (typeof f.then == "function") {
            W = f, f = ae === Sn ? Yi() : null, i = ku(i, o, f).ping, W.then(i, i), o.formatContext = T, o.context = R, o.keyPath = S, o.treeContext = Y, o.componentStack = I, kl(R);
            return;
          }
          if (f.message === "Maximum call stack size exceeded") {
            W = ae === Sn ? Yi() : null, W = ku(i, o, W), i.pingedTasks.push(W), o.formatContext = T, o.context = R, o.keyPath = S, o.treeContext = Y, o.componentStack = I, kl(R);
            return;
          }
        }
      }
    }
    throw o.formatContext = T, o.context = R, o.keyPath = S, o.treeContext = Y, kl(R), f;
  }
  function bs(i) {
    var o = i.blockedBoundary, f = i.blockedSegment;
    f !== null && (f.status = 3, wi(this, o, i.row, f));
  }
  function Il(i, o, f, g, T, R) {
    for (var S = 0; S < f.length; S++) {
      var Y = f[S];
      if (Y.length === 4)
        Il(
          i,
          o,
          Y[2],
          Y[3],
          T,
          R
        );
      else {
        Y = Y[5];
        var I = i, W = R, q = jo(
          I,
          null,
          /* @__PURE__ */ new Set(),
          null,
          null
        );
        q.parentFlushed = !0, q.rootSegmentID = Y, q.status = 4, q.errorDigest = W, q.parentFlushed && I.clientRenderedBoundaries.push(q);
      }
    }
    if (f.length = 0, g !== null) {
      if (o === null) throw Error(G(487));
      if (o.status !== 4 && (o.status = 4, o.errorDigest = R, o.parentFlushed && i.clientRenderedBoundaries.push(o)), typeof g == "object") for (var Q in g) delete g[Q];
    }
  }
  function $u(i, o, f) {
    var g = i.blockedBoundary, T = i.blockedSegment;
    if (T !== null) {
      if (T.status === 6) return;
      T.status = 3;
    }
    var R = qr(i.componentStack);
    if (g === null) {
      if (o.status !== 13 && o.status !== 14) {
        if (g = i.replay, g === null) {
          o.trackedPostpones !== null && T !== null ? (g = o.trackedPostpones, Tn(o, f, R), ka(o, g, i, T), wi(o, null, i.row, T)) : (Tn(o, f, R), jr(o, f));
          return;
        }
        g.pendingTasks--, g.pendingTasks === 0 && 0 < g.nodes.length && (T = Tn(o, f, R), Il(
          o,
          null,
          g.nodes,
          g.slots,
          f,
          T
        )), o.pendingRootTasks--, o.pendingRootTasks === 0 && Kl(o);
      }
    } else {
      var S = o.trackedPostpones;
      if (g.status !== 4) {
        if (S !== null && T !== null)
          return Tn(o, f, R), ka(o, S, i, T), g.fallbackAbortableTasks.forEach(function(Y) {
            return $u(Y, o, f);
          }), g.fallbackAbortableTasks.clear(), wi(o, g, i.row, T);
        g.status = 4, T = Tn(o, f, R), g.status = 4, g.errorDigest = T, Sa(o, g), g.parentFlushed && o.clientRenderedBoundaries.push(g);
      }
      g.pendingTasks--, T = g.row, T !== null && --T.pendingTasks === 0 && Mn(o, T), g.fallbackAbortableTasks.forEach(function(Y) {
        return $u(Y, o, f);
      }), g.fallbackAbortableTasks.clear();
    }
    i = i.row, i !== null && --i.pendingTasks === 0 && Mn(o, i), o.allPendingTasks--, o.allPendingTasks === 0 && rc(o);
  }
  function Su(i, o) {
    try {
      var f = i.renderState, g = f.onHeaders;
      if (g) {
        var T = f.headers;
        if (T) {
          f.headers = null;
          var R = T.preconnects;
          if (T.fontPreloads && (R && (R += ", "), R += T.fontPreloads), T.highImagePreloads && (R && (R += ", "), R += T.highImagePreloads), !o) {
            var S = f.styles.values(), Y = S.next();
            e: for (; 0 < T.remainingCapacity && !Y.done; Y = S.next())
              for (var I = Y.value.sheets.values(), W = I.next(); 0 < T.remainingCapacity && !W.done; W = I.next()) {
                var q = W.value, Q = q.props, ae = Q.href, ve = q.props, De = In(ve.href, "style", {
                  crossOrigin: ve.crossOrigin,
                  integrity: ve.integrity,
                  nonce: ve.nonce,
                  type: ve.type,
                  fetchPriority: ve.fetchPriority,
                  referrerPolicy: ve.referrerPolicy,
                  media: ve.media
                });
                if (0 <= (T.remainingCapacity -= De.length + 2))
                  f.resets.style[ae] = Ct, R && (R += ", "), R += De, f.resets.style[ae] = typeof Q.crossOrigin == "string" || typeof Q.integrity == "string" ? [Q.crossOrigin, Q.integrity] : Ct;
                else break e;
              }
          }
          g(R ? { Link: R } : {});
        }
      }
    } catch (ke) {
      Tn(i, ke, {});
    }
  }
  function Kl(i) {
    i.trackedPostpones === null && Su(i, !0), i.trackedPostpones === null && Fa(i), i.onShellError = St, i = i.onShellReady, i();
  }
  function rc(i) {
    Su(
      i,
      i.trackedPostpones === null ? !0 : i.completedRootSegment === null || i.completedRootSegment.status !== 5
    ), Fa(i), i = i.onAllReady, i();
  }
  function Yt(i, o) {
    if (o.chunks.length === 0 && o.children.length === 1 && o.children[0].boundary === null && o.children[0].id === -1) {
      var f = o.children[0];
      f.id = o.id, f.parentFlushed = !0, f.status !== 1 && f.status !== 3 && f.status !== 4 || Yt(i, f);
    } else i.completedSegments.push(o);
  }
  function wi(i, o, f, g) {
    if (f !== null && (--f.pendingTasks === 0 ? Mn(i, f) : f.together && Lc(i, f)), i.allPendingTasks--, o === null) {
      if (g !== null && g.parentFlushed) {
        if (i.completedRootSegment !== null)
          throw Error(G(389));
        i.completedRootSegment = g;
      }
      i.pendingRootTasks--, i.pendingRootTasks === 0 && Kl(i);
    } else if (o.pendingTasks--, o.status !== 4)
      if (o.pendingTasks === 0) {
        if (o.status === 0 && (o.status = 1), g !== null && g.parentFlushed && (g.status === 1 || g.status === 3) && Yt(o, g), o.parentFlushed && i.completedBoundaries.push(o), o.status === 1)
          f = o.row, f !== null && ro(f.hoistables, o.contentState), Ji(i, o) || (o.fallbackAbortableTasks.forEach(bs, i), o.fallbackAbortableTasks.clear(), f !== null && --f.pendingTasks === 0 && Mn(i, f)), i.pendingRootTasks === 0 && i.trackedPostpones === null && o.contentPreamble !== null && Fa(i);
        else if (o.status === 5 && (o = o.row, o !== null)) {
          if (i.trackedPostpones !== null) {
            f = i.trackedPostpones;
            var T = o.next;
            if (T !== null && (g = T.boundaries, g !== null))
              for (T.boundaries = null, T = 0; T < g.length; T++) {
                var R = g[T];
                Vi(i, f, R), wi(i, R, null, null);
              }
          }
          --o.pendingTasks === 0 && Mn(i, o);
        }
      } else
        g === null || !g.parentFlushed || g.status !== 1 && g.status !== 3 || (Yt(o, g), o.completedSegments.length === 1 && o.parentFlushed && i.partialBoundaries.push(o)), o = o.row, o !== null && o.together && Lc(i, o);
    i.allPendingTasks === 0 && rc(i);
  }
  function es(i) {
    if (i.status !== 14 && i.status !== 13) {
      var o = io, f = de.H;
      de.H = ju;
      var g = de.A;
      de.A = qo;
      var T = ot;
      ot = i;
      var R = Gi;
      Gi = i.resumableState;
      try {
        var S = i.pingedTasks, Y;
        for (Y = 0; Y < S.length; Y++) {
          var I = S[Y], W = i, q = I.blockedSegment;
          if (q === null) {
            var Q = W;
            if (I.replay.pendingTasks !== 0) {
              kl(I.context);
              try {
                if (typeof I.replay.slots == "number" ? tc(
                  Q,
                  I,
                  I.replay.slots,
                  I.node,
                  I.childIndex
                ) : On(Q, I), I.replay.pendingTasks === 1 && 0 < I.replay.nodes.length)
                  throw Error(G(488));
                I.replay.pendingTasks--, I.abortSet.delete(I), wi(
                  Q,
                  I.blockedBoundary,
                  I.row,
                  null
                );
              } catch (Ae) {
                Ea();
                var ae = Ae === Sn ? Sl() : Ae;
                if (typeof ae == "object" && ae !== null && typeof ae.then == "function") {
                  var ve = I.ping;
                  ae.then(ve, ve), I.thenableState = Ae === Sn ? Yi() : null;
                } else {
                  I.replay.pendingTasks--, I.abortSet.delete(I);
                  var De = qr(I.componentStack);
                  W = void 0;
                  var ke = Q, xe = I.blockedBoundary, me = Q.status === 12 ? Q.fatalError : ae, Ce = I.replay.nodes, _n = I.replay.slots;
                  W = Tn(
                    ke,
                    me,
                    De
                  ), Il(
                    ke,
                    xe,
                    Ce,
                    _n,
                    me,
                    W
                  ), Q.pendingRootTasks--, Q.pendingRootTasks === 0 && Kl(Q), Q.allPendingTasks--, Q.allPendingTasks === 0 && rc(Q);
                }
              } finally {
              }
            }
          } else if (Q = void 0, ke = q, ke.status === 0) {
            ke.status = 6, kl(I.context);
            var Le = ke.children.length, Je = ke.chunks.length;
            try {
              On(W, I), Gl(
                ke.chunks,
                W.renderState,
                ke.lastPushedText,
                ke.textEmbedded
              ), I.abortSet.delete(I), ke.status = 1, wi(
                W,
                I.blockedBoundary,
                I.row,
                ke
              );
            } catch (Ae) {
              Ea(), ke.children.length = Le, ke.chunks.length = Je;
              var on = Ae === Sn ? Sl() : W.status === 12 ? W.fatalError : Ae;
              if (W.status === 12 && W.trackedPostpones !== null) {
                var Me = W.trackedPostpones, Hn = qr(I.componentStack);
                I.abortSet.delete(I), Tn(W, on, Hn), ka(W, Me, I, ke), wi(
                  W,
                  I.blockedBoundary,
                  I.row,
                  ke
                );
              } else if (typeof on == "object" && on !== null && typeof on.then == "function") {
                ke.status = 0, I.thenableState = Ae === Sn ? Yi() : null;
                var nn = I.ping;
                on.then(nn, nn);
              } else {
                var nr = qr(I.componentStack);
                I.abortSet.delete(I), ke.status = 4;
                var sn = I.blockedBoundary, Ke = I.row;
                if (Ke !== null && --Ke.pendingTasks === 0 && Mn(W, Ke), W.allPendingTasks--, Q = Tn(
                  W,
                  on,
                  nr
                ), sn === null) jr(W, on);
                else if (sn.pendingTasks--, sn.status !== 4) {
                  sn.status = 4, sn.errorDigest = Q, Sa(W, sn);
                  var tn = sn.row;
                  tn !== null && --tn.pendingTasks === 0 && Mn(W, tn), sn.parentFlushed && W.clientRenderedBoundaries.push(sn), W.pendingRootTasks === 0 && W.trackedPostpones === null && sn.contentPreamble !== null && Fa(W);
                }
                W.allPendingTasks === 0 && rc(W);
              }
            } finally {
            }
          }
        }
        S.splice(0, Y), i.destination !== null && Wc(i, i.destination);
      } catch (Ae) {
        Tn(i, Ae, {}), jr(i, Ae);
      } finally {
        Gi = R, de.H = f, de.A = g, f === ju && kl(o), ot = T;
      }
    }
  }
  function Pa(i, o, f) {
    o.preambleChildren.length && f.push(o.preambleChildren);
    for (var g = !1, T = 0; T < o.children.length; T++)
      g = Nc(
        i,
        o.children[T],
        f
      ) || g;
    return g;
  }
  function Nc(i, o, f) {
    var g = o.boundary;
    if (g === null)
      return Pa(
        i,
        o,
        f
      );
    var T = g.contentPreamble, R = g.fallbackPreamble;
    if (T === null || R === null) return !1;
    switch (g.status) {
      case 1:
        if (Zu(i.renderState, T), i.byteSize += g.byteSize, o = g.completedSegments[0], !o) throw Error(G(391));
        return Pa(
          i,
          o,
          f
        );
      case 5:
        if (i.trackedPostpones !== null) return !0;
      case 4:
        if (o.status === 1)
          return Zu(i.renderState, R), Pa(
            i,
            o,
            f
          );
      default:
        return !0;
    }
  }
  function Fa(i) {
    if (i.completedRootSegment && i.completedPreambleSegments === null) {
      var o = [], f = i.byteSize, g = Nc(
        i,
        i.completedRootSegment,
        o
      ), T = i.renderState.preamble;
      g === !1 || T.headChunks && T.bodyChunks ? i.completedPreambleSegments = o : i.byteSize = f;
    }
  }
  function Gt(i, o, f, g) {
    switch (f.parentFlushed = !0, f.status) {
      case 0:
        f.id = i.nextSegmentId++;
      case 5:
        return g = f.id, f.lastPushedText = !1, f.textEmbedded = !1, i = i.renderState, o.push('<template id="'), o.push(i.placeholderPrefix), i = g.toString(16), o.push(i), o.push('"></template>');
      case 1:
        f.status = 2;
        var T = !0, R = f.chunks, S = 0;
        f = f.children;
        for (var Y = 0; Y < f.length; Y++) {
          for (T = f[Y]; S < T.index; S++)
            o.push(R[S]);
          T = Ki(i, o, T, g);
        }
        for (; S < R.length - 1; S++)
          o.push(R[S]);
        return S < R.length && (T = o.push(R[S])), T;
      case 3:
        return !0;
      default:
        throw Error(G(390));
    }
  }
  var Dt = 0;
  function Ki(i, o, f, g) {
    var T = f.boundary;
    if (T === null)
      return Gt(i, o, f, g);
    if (T.parentFlushed = !0, T.status === 4) {
      var R = T.row;
      return R !== null && --R.pendingTasks === 0 && Mn(i, R), i.renderState.generateStaticMarkup || (T = T.errorDigest, o.push("<!--$!-->"), o.push("<template"), T && (o.push(' data-dgst="'), T = ue(T), o.push(T), o.push('"')), o.push("></template>")), Gt(i, o, f, g), i = i.renderState.generateStaticMarkup ? !0 : o.push("<!--/$-->"), i;
    }
    if (T.status !== 1)
      return T.status === 0 && (T.rootSegmentID = i.nextSegmentId++), 0 < T.completedSegments.length && i.partialBoundaries.push(T), ll(
        o,
        i.renderState,
        T.rootSegmentID
      ), g && ro(g, T.fallbackState), Gt(i, o, f, g), o.push("<!--/$-->");
    if (!go && Ji(i, T) && Dt + T.byteSize > i.progressiveChunkSize)
      return T.rootSegmentID = i.nextSegmentId++, i.completedBoundaries.push(T), ll(
        o,
        i.renderState,
        T.rootSegmentID
      ), Gt(i, o, f, g), o.push("<!--/$-->");
    if (Dt += T.byteSize, g && ro(g, T.contentState), f = T.row, f !== null && Ji(i, T) && --f.pendingTasks === 0 && Mn(i, f), i.renderState.generateStaticMarkup || o.push("<!--$-->"), f = T.completedSegments, f.length !== 1) throw Error(G(391));
    return Ki(i, o, f[0], g), i = i.renderState.generateStaticMarkup ? !0 : o.push("<!--/$-->"), i;
  }
  function Pn(i, o, f, g) {
    return Hl(
      o,
      i.renderState,
      f.parentFormatContext,
      f.id
    ), Ki(i, o, f, g), Ju(o, f.parentFormatContext);
  }
  function gn(i, o, f) {
    Dt = f.byteSize;
    for (var g = f.completedSegments, T = 0; T < g.length; T++)
      ma(
        i,
        o,
        f,
        g[T]
      );
    g.length = 0, g = f.row, g !== null && Ji(i, f) && --g.pendingTasks === 0 && Mn(i, g), Ho(
      o,
      f.contentState,
      i.renderState
    ), g = i.resumableState, i = i.renderState, T = f.rootSegmentID, f = f.contentState;
    var R = i.stylesToHoist;
    return i.stylesToHoist = !1, o.push(i.startInlineScript), o.push(">"), R ? (!(g.instructions & 4) && (g.instructions |= 4, o.push(
      '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};'
    )), !(g.instructions & 2) && (g.instructions |= 2, o.push(
      `$RB=[];$RV=function(a){$RT=performance.now();for(var b=0;b<a.length;b+=2){var c=a[b],e=a[b+1];null!==e.parentNode&&e.parentNode.removeChild(e);var f=c.parentNode;if(f){var g=c.previousSibling,h=0;do{if(c&&8===c.nodeType){var d=c.data;if("/$"===d||"/&"===d)if(0===h)break;else h--;else"$"!==d&&"$?"!==d&&"$~"!==d&&"$!"!==d&&"&"!==d||h++}d=c.nextSibling;f.removeChild(c);c=d}while(c);for(;e.firstChild;)f.insertBefore(e.firstChild,c);g.data="$";g._reactRetry&&requestAnimationFrame(g._reactRetry)}}a.length=0};
$RC=function(a,b){if(b=document.getElementById(b))(a=document.getElementById(a))?(a.previousSibling.data="$~",$RB.push(a,b),2===$RB.length&&("number"!==typeof $RT?requestAnimationFrame($RV.bind(null,$RB)):(a=performance.now(),setTimeout($RV.bind(null,$RB),2300>a&&2E3<a?2300-a:$RT+300-a)))):b.parentNode.removeChild(b)};`
    )), g.instructions & 8 ? o.push('$RR("') : (g.instructions |= 8, o.push(
      `$RM=new Map;$RR=function(n,w,p){function u(q){this._p=null;q()}for(var r=new Map,t=document,h,b,e=t.querySelectorAll("link[data-precedence],style[data-precedence]"),v=[],k=0;b=e[k++];)"not all"===b.getAttribute("media")?v.push(b):("LINK"===b.tagName&&$RM.set(b.getAttribute("href"),b),r.set(b.dataset.precedence,h=b));e=0;b=[];var l,a;for(k=!0;;){if(k){var f=p[e++];if(!f){k=!1;e=0;continue}var c=!1,m=0;var d=f[m++];if(a=$RM.get(d)){var g=a._p;c=!0}else{a=t.createElement("link");a.href=d;a.rel=
"stylesheet";for(a.dataset.precedence=l=f[m++];g=f[m++];)a.setAttribute(g,f[m++]);g=a._p=new Promise(function(q,x){a.onload=u.bind(a,q);a.onerror=u.bind(a,x)});$RM.set(d,a)}d=a.getAttribute("media");!g||d&&!matchMedia(d).matches||b.push(g);if(c)continue}else{a=v[e++];if(!a)break;l=a.getAttribute("data-precedence");a.removeAttribute("media")}c=r.get(l)||h;c===h&&(h=a);r.set(l,a);c?c.parentNode.insertBefore(a,c.nextSibling):(c=t.head,c.insertBefore(a,c.firstChild))}if(p=document.getElementById(n))p.previousSibling.data=
"$~";Promise.all(b).then($RC.bind(null,n,w),$RX.bind(null,n,"CSS failed to load"))};$RR("`
    ))) : (!(g.instructions & 2) && (g.instructions |= 2, o.push(
      `$RB=[];$RV=function(a){$RT=performance.now();for(var b=0;b<a.length;b+=2){var c=a[b],e=a[b+1];null!==e.parentNode&&e.parentNode.removeChild(e);var f=c.parentNode;if(f){var g=c.previousSibling,h=0;do{if(c&&8===c.nodeType){var d=c.data;if("/$"===d||"/&"===d)if(0===h)break;else h--;else"$"!==d&&"$?"!==d&&"$~"!==d&&"$!"!==d&&"&"!==d||h++}d=c.nextSibling;f.removeChild(c);c=d}while(c);for(;e.firstChild;)f.insertBefore(e.firstChild,c);g.data="$";g._reactRetry&&requestAnimationFrame(g._reactRetry)}}a.length=0};
$RC=function(a,b){if(b=document.getElementById(b))(a=document.getElementById(a))?(a.previousSibling.data="$~",$RB.push(a,b),2===$RB.length&&("number"!==typeof $RT?requestAnimationFrame($RV.bind(null,$RB)):(a=performance.now(),setTimeout($RV.bind(null,$RB),2300>a&&2E3<a?2300-a:$RT+300-a)))):b.parentNode.removeChild(b)};`
    )), o.push('$RC("')), g = T.toString(16), o.push(i.boundaryPrefix), o.push(g), o.push('","'), o.push(i.segmentPrefix), o.push(g), R ? (o.push('",'), Ul(o, f)) : o.push('"'), f = o.push(")<\/script>"), bu(o, i) && f;
  }
  function ma(i, o, f, g) {
    if (g.status === 2) return !0;
    var T = f.contentState, R = g.id;
    if (R === -1) {
      if ((g.id = f.rootSegmentID) === -1)
        throw Error(G(392));
      return Pn(i, o, g, T);
    }
    return R === f.rootSegmentID ? Pn(i, o, g, T) : (Pn(i, o, g, T), f = i.resumableState, i = i.renderState, o.push(i.startInlineScript), o.push(">"), f.instructions & 1 ? o.push('$RS("') : (f.instructions |= 1, o.push(
      '$RS=function(a,b){a=document.getElementById(a);b=document.getElementById(b);for(a.parentNode.removeChild(a);a.firstChild;)b.parentNode.insertBefore(a.firstChild,b);b.parentNode.removeChild(b)};$RS("'
    )), o.push(i.segmentPrefix), R = R.toString(16), o.push(R), o.push('","'), o.push(i.placeholderPrefix), o.push(R), o = o.push('")<\/script>'), o);
  }
  var go = !1;
  function Wc(i, o) {
    try {
      if (!(0 < i.pendingRootTasks)) {
        var f, g = i.completedRootSegment;
        if (g !== null) {
          if (g.status === 5) return;
          var T = i.completedPreambleSegments;
          if (T === null) return;
          Dt = i.byteSize;
          var R = i.resumableState, S = i.renderState, Y = S.preamble, I = Y.htmlChunks, W = Y.headChunks, q;
          if (I) {
            for (q = 0; q < I.length; q++)
              o.push(I[q]);
            if (W)
              for (q = 0; q < W.length; q++)
                o.push(W[q]);
            else {
              var Q = Nn("head");
              o.push(Q), o.push(">");
            }
          } else if (W)
            for (q = 0; q < W.length; q++)
              o.push(W[q]);
          var ae = S.charsetChunks;
          for (q = 0; q < ae.length; q++)
            o.push(ae[q]);
          ae.length = 0, S.preconnects.forEach(Jr, o), S.preconnects.clear();
          var ve = S.viewportChunks;
          for (q = 0; q < ve.length; q++)
            o.push(ve[q]);
          ve.length = 0, S.fontPreloads.forEach(Jr, o), S.fontPreloads.clear(), S.highImagePreloads.forEach(Jr, o), S.highImagePreloads.clear(), _e = S, S.styles.forEach(Ye, o), _e = null;
          var De = S.importMapChunks;
          for (q = 0; q < De.length; q++)
            o.push(De[q]);
          De.length = 0, S.bootstrapScripts.forEach(Jr, o), S.scripts.forEach(Jr, o), S.scripts.clear(), S.bulkPreloads.forEach(Jr, o), S.bulkPreloads.clear(), R.instructions |= 32;
          var ke = S.hoistableChunks;
          for (q = 0; q < ke.length; q++)
            o.push(ke[q]);
          for (R = ke.length = 0; R < T.length; R++) {
            var xe = T[R];
            for (S = 0; S < xe.length; S++)
              Ki(i, o, xe[S], null);
          }
          var me = i.renderState.preamble, Ce = me.headChunks;
          if (me.htmlChunks || Ce) {
            var _n = va("head");
            o.push(_n);
          }
          var Le = me.bodyChunks;
          if (Le)
            for (T = 0; T < Le.length; T++)
              o.push(Le[T]);
          Ki(i, o, g, null), i.completedRootSegment = null;
          var Je = i.renderState;
          if (i.allPendingTasks !== 0 || i.clientRenderedBoundaries.length !== 0 || i.completedBoundaries.length !== 0 || i.trackedPostpones !== null && (i.trackedPostpones.rootNodes.length !== 0 || i.trackedPostpones.rootSlots !== null)) {
            var on = i.resumableState;
            if (!(on.instructions & 64)) {
              if (on.instructions |= 64, o.push(Je.startInlineScript), !(on.instructions & 32)) {
                on.instructions |= 32;
                var Me = "_" + on.idPrefix + "R_";
                o.push(' id="');
                var Hn = ue(Me);
                o.push(Hn), o.push('"');
              }
              o.push(">"), o.push(
                "requestAnimationFrame(function(){$RT=performance.now()});"
              ), o.push("<\/script>");
            }
          }
          bu(o, Je);
        }
        var nn = i.renderState;
        g = 0;
        var nr = nn.viewportChunks;
        for (g = 0; g < nr.length; g++)
          o.push(nr[g]);
        nr.length = 0, nn.preconnects.forEach(Jr, o), nn.preconnects.clear(), nn.fontPreloads.forEach(Jr, o), nn.fontPreloads.clear(), nn.highImagePreloads.forEach(
          Jr,
          o
        ), nn.highImagePreloads.clear(), nn.styles.forEach(Yo, o), nn.scripts.forEach(Jr, o), nn.scripts.clear(), nn.bulkPreloads.forEach(Jr, o), nn.bulkPreloads.clear();
        var sn = nn.hoistableChunks;
        for (g = 0; g < sn.length; g++)
          o.push(sn[g]);
        sn.length = 0;
        var Ke = i.clientRenderedBoundaries;
        for (f = 0; f < Ke.length; f++) {
          var tn = Ke[f];
          nn = o;
          var Ae = i.resumableState, rn = i.renderState, Ft = tn.rootSegmentID, Be = tn.errorDigest;
          nn.push(rn.startInlineScript), nn.push(">"), Ae.instructions & 4 ? nn.push('$RX("') : (Ae.instructions |= 4, nn.push(
            '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};;$RX("'
          )), nn.push(rn.boundaryPrefix);
          var qi = Ft.toString(16);
          if (nn.push(qi), nn.push('"'), Be) {
            nn.push(",");
            var Ei = qn(
              Be || ""
            );
            nn.push(Ei);
          }
          var tr = nn.push(")<\/script>");
          if (!tr) {
            i.destination = null, f++, Ke.splice(0, f);
            return;
          }
        }
        Ke.splice(0, f);
        var il = i.completedBoundaries;
        for (f = 0; f < il.length; f++)
          if (!gn(i, o, il[f])) {
            i.destination = null, f++, il.splice(0, f);
            return;
          }
        il.splice(0, f), go = !0;
        var al = i.partialBoundaries;
        for (f = 0; f < al.length; f++) {
          var ol = al[f];
          e: {
            Ke = i, tn = o, Dt = ol.byteSize;
            var ql = ol.completedSegments;
            for (tr = 0; tr < ql.length; tr++)
              if (!ma(
                Ke,
                tn,
                ol,
                ql[tr]
              )) {
                tr++, ql.splice(0, tr);
                var Ri = !1;
                break e;
              }
            ql.splice(0, tr);
            var wt = ol.row;
            wt !== null && wt.together && ol.pendingTasks === 1 && (wt.pendingTasks === 1 ? Al(
              Ke,
              wt,
              wt.hoistables
            ) : wt.pendingTasks--), Ri = Ho(
              tn,
              ol.contentState,
              Ke.renderState
            );
          }
          if (!Ri) {
            i.destination = null, f++, al.splice(0, f);
            return;
          }
        }
        al.splice(0, f), go = !1;
        var Aa = i.completedBoundaries;
        for (f = 0; f < Aa.length; f++)
          if (!gn(i, o, Aa[f])) {
            i.destination = null, f++, Aa.splice(0, f);
            return;
          }
        Aa.splice(0, f);
      }
    } finally {
      go = !1, i.allPendingTasks === 0 && i.clientRenderedBoundaries.length === 0 && i.completedBoundaries.length === 0 && (i.flushScheduled = !1, f = i.resumableState, f.hasBody && (al = va("body"), o.push(al)), f.hasHtml && (f = va("html"), o.push(f)), i.status = 14, o.push(null), i.destination = null);
    }
  }
  function vo(i) {
    if (i.flushScheduled === !1 && i.pingedTasks.length === 0 && i.destination !== null) {
      i.flushScheduled = !0;
      var o = i.destination;
      o ? Wc(i, o) : i.flushScheduled = !1;
    }
  }
  function Pu(i, o) {
    if (i.status === 13)
      i.status = 14, o.destroy(i.fatalError);
    else if (i.status !== 14 && i.destination === null) {
      i.destination = o;
      try {
        Wc(i, o);
      } catch (f) {
        Tn(i, f, {}), jr(i, f);
      }
    }
  }
  function ys(i, o) {
    (i.status === 11 || i.status === 10) && (i.status = 12);
    try {
      var f = i.abortableTasks;
      if (0 < f.size) {
        var g = o === void 0 ? Error(G(432)) : typeof o == "object" && o !== null && typeof o.then == "function" ? Error(G(530)) : o;
        i.fatalError = g, f.forEach(function(T) {
          return $u(T, i, g);
        }), f.clear();
      }
      i.destination !== null && Wc(i, i.destination);
    } catch (T) {
      Tn(i, T, {}), jr(i, T);
    }
  }
  function Wn(i, o, f) {
    if (o === null) f.rootNodes.push(i);
    else {
      var g = f.workingMap, T = g.get(o);
      T === void 0 && (T = [o[1], o[2], [], null], g.set(o, T), Wn(T, o[0], f)), T[2].push(i);
    }
  }
  function pi() {
  }
  function ns(i, o, f, g) {
    var T = !1, R = null, S = "", Y = !1;
    if (o = Mt(o ? o.identifierPrefix : void 0), i = Ca(
      i,
      o,
      lo(o, f),
      rt(0, null, 0, null),
      1 / 0,
      pi,
      void 0,
      function() {
        Y = !0;
      },
      void 0,
      void 0,
      void 0
    ), i.flushScheduled = i.destination !== null, es(i), i.status === 10 && (i.status = 11), i.trackedPostpones === null && Su(i, i.pendingRootTasks === 0), ys(i, g), Pu(i, {
      push: function(I) {
        return I !== null && (S += I), !0;
      },
      destroy: function(I) {
        T = !0, R = I;
      }
    }), T && R !== g) throw R;
    if (!Y) throw Error(G(426));
    return S;
  }
  return Js.renderToStaticMarkup = function(i, o) {
    return ns(
      i,
      o,
      !0,
      'The server used "renderToStaticMarkup" which does not support Suspense. If you intended to have the server wait for the suspended component please switch to "renderToReadableStream" which supports Suspense on the server'
    );
  }, Js.renderToString = function(i, o) {
    return ns(
      i,
      o,
      !1,
      'The server used "renderToString" which does not support Suspense. If you intended for this Suspense boundary to render the fallback content on the server consider throwing an Error somewhere within the Suspense boundary. If you intended to have the server wait for the suspended component please switch to "renderToReadableStream" which supports Suspense on the server'
    );
  }, Js.version = "19.2.4", Js;
}
var Ds = {};
/**
 * @license React
 * react-dom-server.browser.production.js
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
var Sf;
function Df() {
  if (Sf) return Ds;
  Sf = 1;
  var Te = ks, Vn = rf;
  function G(l) {
    var a = "https://react.dev/errors/" + l;
    if (1 < arguments.length) {
      a += "?args[]=" + encodeURIComponent(arguments[1]);
      for (var s = 2; s < arguments.length; s++)
        a += "&args[]=" + encodeURIComponent(arguments[s]);
    }
    return "Minified React error #" + l + "; visit " + a + " for the full message or use the non-minified dev environment for full errors and additional helpful warnings.";
  }
  var vt = Symbol.for("react.transitional.element"), Qn = Symbol.for("react.portal"), kn = Symbol.for("react.fragment"), Qe = Symbol.for("react.strict_mode"), Kn = Symbol.for("react.profiler"), Nl = Symbol.for("react.consumer"), Et = Symbol.for("react.context"), jt = Symbol.for("react.forward_ref"), H = Symbol.for("react.suspense"), pe = Symbol.for("react.suspense_list"), Ar = Symbol.for("react.memo"), ne = Symbol.for("react.lazy"), N = Symbol.for("react.scope"), Zr = Symbol.for("react.activity"), zo = Symbol.for("react.legacy_hidden"), _i = Symbol.for("react.memo_cache_sentinel"), Bo = Symbol.for("react.view_transition"), Di = Symbol.iterator;
  function Nt(l) {
    return l === null || typeof l != "object" ? null : (l = Di && l[Di] || l["@@iterator"], typeof l == "function" ? l : null);
  }
  var Ee = Array.isArray;
  function Ir(l, a) {
    var s = l.length & 3, v = l.length - s, x = a;
    for (a = 0; a < v; ) {
      var E = l.charCodeAt(a) & 255 | (l.charCodeAt(++a) & 255) << 8 | (l.charCodeAt(++a) & 255) << 16 | (l.charCodeAt(++a) & 255) << 24;
      ++a, E = 3432918353 * (E & 65535) + ((3432918353 * (E >>> 16) & 65535) << 16) & 4294967295, E = E << 15 | E >>> 17, E = 461845907 * (E & 65535) + ((461845907 * (E >>> 16) & 65535) << 16) & 4294967295, x ^= E, x = x << 13 | x >>> 19, x = 5 * (x & 65535) + ((5 * (x >>> 16) & 65535) << 16) & 4294967295, x = (x & 65535) + 27492 + (((x >>> 16) + 58964 & 65535) << 16);
    }
    switch (E = 0, s) {
      case 3:
        E ^= (l.charCodeAt(a + 2) & 255) << 16;
      case 2:
        E ^= (l.charCodeAt(a + 1) & 255) << 8;
      case 1:
        E ^= l.charCodeAt(a) & 255, E = 3432918353 * (E & 65535) + ((3432918353 * (E >>> 16) & 65535) << 16) & 4294967295, E = E << 15 | E >>> 17, x ^= 461845907 * (E & 65535) + ((461845907 * (E >>> 16) & 65535) << 16) & 4294967295;
    }
    return x ^= l.length, x ^= x >>> 16, x = 2246822507 * (x & 65535) + ((2246822507 * (x >>> 16) & 65535) << 16) & 4294967295, x ^= x >>> 13, x = 3266489909 * (x & 65535) + ((3266489909 * (x >>> 16) & 65535) << 16) & 4294967295, (x ^ x >>> 16) >>> 0;
  }
  var Rt = new MessageChannel(), dn = [];
  Rt.port1.onmessage = function() {
    var l = dn.shift();
    l && l();
  };
  function bt(l) {
    dn.push(l), Rt.port2.postMessage(null);
  }
  function kc(l) {
    setTimeout(function() {
      throw l;
    });
  }
  var eo = Promise, Li = typeof queueMicrotask == "function" ? queueMicrotask : function(l) {
    eo.resolve(null).then(l).catch(kc);
  }, Wt = null, ge = 0;
  function V(l, a) {
    if (a.byteLength !== 0)
      if (2048 < a.byteLength)
        0 < ge && (l.enqueue(
          new Uint8Array(Wt.buffer, 0, ge)
        ), Wt = new Uint8Array(2048), ge = 0), l.enqueue(a);
      else {
        var s = Wt.length - ge;
        s < a.byteLength && (s === 0 ? l.enqueue(Wt) : (Wt.set(a.subarray(0, s), ge), l.enqueue(Wt), a = a.subarray(s)), Wt = new Uint8Array(2048), ge = 0), Wt.set(a, ge), ge += a.byteLength;
      }
  }
  function ue(l, a) {
    return V(l, a), !0;
  }
  function Wl(l) {
    Wt && 0 < ge && (l.enqueue(new Uint8Array(Wt.buffer, 0, ge)), Wt = null, ge = 0);
  }
  var no = new TextEncoder();
  function le(l) {
    return no.encode(l);
  }
  function F(l) {
    return no.encode(l);
  }
  function de(l) {
    return l.byteLength;
  }
  function We(l, a) {
    typeof l.error == "function" ? l.error(a) : l.close();
  }
  var Ze = Object.assign, He = Object.prototype.hasOwnProperty, Ct = RegExp(
    "^[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
  ), _e = {}, pl = {};
  function Mr(l) {
    return He.call(pl, l) ? !0 : He.call(_e, l) ? !1 : Ct.test(l) ? pl[l] = !0 : (_e[l] = !0, !1);
  }
  var Mt = new Set(
    "animationIterationCount aspectRatio borderImageOutset borderImageSlice borderImageWidth boxFlex boxFlexGroup boxOrdinalGroup columnCount columns flex flexGrow flexPositive flexShrink flexNegative flexOrder gridArea gridRow gridRowEnd gridRowSpan gridRowStart gridColumn gridColumnEnd gridColumnSpan gridColumnStart fontWeight lineClamp lineHeight opacity order orphans scale tabSize widows zIndex zoom fillOpacity floodOpacity stopOpacity strokeDasharray strokeDashoffset strokeMiterlimit strokeOpacity strokeWidth MozAnimationIterationCount MozBoxFlex MozBoxFlexGroup MozLineClamp msAnimationIterationCount msFlex msZoom msFlexGrow msFlexNegative msFlexOrder msFlexPositive msFlexShrink msGridColumn msGridColumnSpan msGridRow msGridRowSpan WebkitAnimationIterationCount WebkitBoxFlex WebKitBoxFlexGroup WebkitBoxOrdinalGroup WebkitColumnCount WebkitColumns WebkitFlex WebkitFlexGrow WebkitFlexPositive WebkitFlexShrink WebkitLineClamp".split(
      " "
    )
  ), rt = /* @__PURE__ */ new Map([
    ["acceptCharset", "accept-charset"],
    ["htmlFor", "for"],
    ["httpEquiv", "http-equiv"],
    ["crossOrigin", "crossorigin"],
    ["accentHeight", "accent-height"],
    ["alignmentBaseline", "alignment-baseline"],
    ["arabicForm", "arabic-form"],
    ["baselineShift", "baseline-shift"],
    ["capHeight", "cap-height"],
    ["clipPath", "clip-path"],
    ["clipRule", "clip-rule"],
    ["colorInterpolation", "color-interpolation"],
    ["colorInterpolationFilters", "color-interpolation-filters"],
    ["colorProfile", "color-profile"],
    ["colorRendering", "color-rendering"],
    ["dominantBaseline", "dominant-baseline"],
    ["enableBackground", "enable-background"],
    ["fillOpacity", "fill-opacity"],
    ["fillRule", "fill-rule"],
    ["floodColor", "flood-color"],
    ["floodOpacity", "flood-opacity"],
    ["fontFamily", "font-family"],
    ["fontSize", "font-size"],
    ["fontSizeAdjust", "font-size-adjust"],
    ["fontStretch", "font-stretch"],
    ["fontStyle", "font-style"],
    ["fontVariant", "font-variant"],
    ["fontWeight", "font-weight"],
    ["glyphName", "glyph-name"],
    ["glyphOrientationHorizontal", "glyph-orientation-horizontal"],
    ["glyphOrientationVertical", "glyph-orientation-vertical"],
    ["horizAdvX", "horiz-adv-x"],
    ["horizOriginX", "horiz-origin-x"],
    ["imageRendering", "image-rendering"],
    ["letterSpacing", "letter-spacing"],
    ["lightingColor", "lighting-color"],
    ["markerEnd", "marker-end"],
    ["markerMid", "marker-mid"],
    ["markerStart", "marker-start"],
    ["overlinePosition", "overline-position"],
    ["overlineThickness", "overline-thickness"],
    ["paintOrder", "paint-order"],
    ["panose-1", "panose-1"],
    ["pointerEvents", "pointer-events"],
    ["renderingIntent", "rendering-intent"],
    ["shapeRendering", "shape-rendering"],
    ["stopColor", "stop-color"],
    ["stopOpacity", "stop-opacity"],
    ["strikethroughPosition", "strikethrough-position"],
    ["strikethroughThickness", "strikethrough-thickness"],
    ["strokeDasharray", "stroke-dasharray"],
    ["strokeDashoffset", "stroke-dashoffset"],
    ["strokeLinecap", "stroke-linecap"],
    ["strokeLinejoin", "stroke-linejoin"],
    ["strokeMiterlimit", "stroke-miterlimit"],
    ["strokeOpacity", "stroke-opacity"],
    ["strokeWidth", "stroke-width"],
    ["textAnchor", "text-anchor"],
    ["textDecoration", "text-decoration"],
    ["textRendering", "text-rendering"],
    ["transformOrigin", "transform-origin"],
    ["underlinePosition", "underline-position"],
    ["underlineThickness", "underline-thickness"],
    ["unicodeBidi", "unicode-bidi"],
    ["unicodeRange", "unicode-range"],
    ["unitsPerEm", "units-per-em"],
    ["vAlphabetic", "v-alphabetic"],
    ["vHanging", "v-hanging"],
    ["vIdeographic", "v-ideographic"],
    ["vMathematical", "v-mathematical"],
    ["vectorEffect", "vector-effect"],
    ["vertAdvY", "vert-adv-y"],
    ["vertOriginX", "vert-origin-x"],
    ["vertOriginY", "vert-origin-y"],
    ["wordSpacing", "word-spacing"],
    ["writingMode", "writing-mode"],
    ["xmlnsXlink", "xmlns:xlink"],
    ["xHeight", "x-height"]
  ]), Sc = /["'&<>]/;
  function Fe(l) {
    if (typeof l == "boolean" || typeof l == "number" || typeof l == "bigint")
      return "" + l;
    l = "" + l;
    var a = Sc.exec(l);
    if (a) {
      var s = "", v, x = 0;
      for (v = a.index; v < l.length; v++) {
        switch (l.charCodeAt(v)) {
          case 34:
            a = "&quot;";
            break;
          case 38:
            a = "&amp;";
            break;
          case 39:
            a = "&#x27;";
            break;
          case 60:
            a = "&lt;";
            break;
          case 62:
            a = "&gt;";
            break;
          default:
            continue;
        }
        x !== v && (s += l.slice(x, v)), x = v + 1, s += a;
      }
      l = x !== v ? s + l.slice(x, v) : s;
    }
    return l;
  }
  var lt = /([A-Z])/g, un = /^ms-/, kt = /^[\u0000-\u001F ]*j[\r\n\t]*a[\r\n\t]*v[\r\n\t]*a[\r\n\t]*s[\r\n\t]*c[\r\n\t]*r[\r\n\t]*i[\r\n\t]*p[\r\n\t]*t[\r\n\t]*:/i;
  function da(l) {
    return kt.test("" + l) ? "javascript:throw new Error('React has blocked a javascript: URL as a security precaution.')" : l;
  }
  var zi = Te.__CLIENT_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, Ot = Vn.__DOM_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, xr = {
    pending: !1,
    data: null,
    method: null,
    action: null
  }, El = Ot.d;
  Ot.d = {
    f: El.f,
    r: El.r,
    D: $u,
    C: Su,
    L: Kl,
    m: rc,
    X: wi,
    S: Yt,
    M: es
  };
  var yt = [], Rl = null;
  F('"></template>');
  var Pc = F("<script"), qe = F("<\/script>"), $t = F('<script src="'), Rn = F('<script type="module" src="'), Xu = F(' nonce="'), et = F(' integrity="'), hu = F(' crossorigin="'), gu = F(' async=""><\/script>'), si = F("<style"), vu = /(<\/|<)(s)(cript)/gi;
  function No(l, a, s, v) {
    return "" + a + (s === "s" ? "\\u0073" : "\\u0053") + v;
  }
  var ha = F(
    '<script type="importmap">'
  ), Bi = F("<\/script>");
  function ga(l, a, s, v, x, E) {
    s = typeof a == "string" ? a : a && a.script;
    var C = s === void 0 ? Pc : F(
      '<script nonce="' + Fe(s) + '"'
    ), _ = typeof a == "string" ? void 0 : a && a.style, m = _ === void 0 ? si : F(
      '<style nonce="' + Fe(_) + '"'
    ), D = l.idPrefix, U = [], Z = l.bootstrapScriptContent, se = l.bootstrapScripts, ce = l.bootstrapModules;
    if (Z !== void 0 && (U.push(C), Vi(U, l), U.push(
      $e,
      le(
        ("" + Z).replace(vu, No)
      ),
      qe
    )), Z = [], v !== void 0 && (Z.push(ha), Z.push(
      le(
        ("" + JSON.stringify(v)).replace(vu, No)
      )
    ), Z.push(Bi)), v = x ? {
      preconnects: "",
      fontPreloads: "",
      highImagePreloads: "",
      remainingCapacity: 2 + (typeof E == "number" ? E : 2e3)
    } : null, x = {
      placeholderPrefix: F(D + "P:"),
      segmentPrefix: F(D + "S:"),
      boundaryPrefix: F(D + "B:"),
      startInlineScript: C,
      startInlineStyle: m,
      preamble: Nn(),
      externalRuntimeScript: null,
      bootstrapChunks: U,
      importMapChunks: Z,
      onHeaders: x,
      headers: v,
      resets: {
        font: {},
        dns: {},
        connect: { default: {}, anonymous: {}, credentials: {} },
        image: {},
        style: {}
      },
      charsetChunks: [],
      viewportChunks: [],
      hoistableChunks: [],
      preconnects: /* @__PURE__ */ new Set(),
      fontPreloads: /* @__PURE__ */ new Set(),
      highImagePreloads: /* @__PURE__ */ new Set(),
      styles: /* @__PURE__ */ new Map(),
      bootstrapScripts: /* @__PURE__ */ new Set(),
      scripts: /* @__PURE__ */ new Set(),
      bulkPreloads: /* @__PURE__ */ new Set(),
      preloads: {
        images: /* @__PURE__ */ new Map(),
        stylesheets: /* @__PURE__ */ new Map(),
        scripts: /* @__PURE__ */ new Map(),
        moduleScripts: /* @__PURE__ */ new Map()
      },
      nonce: { script: s, style: _ },
      hoistableState: null,
      stylesToHoist: !1
    }, se !== void 0)
      for (v = 0; v < se.length; v++)
        D = se[v], _ = C = void 0, m = {
          rel: "preload",
          as: "script",
          fetchPriority: "low",
          nonce: a
        }, typeof D == "string" ? m.href = E = D : (m.href = E = D.src, m.integrity = _ = typeof D.integrity == "string" ? D.integrity : void 0, m.crossOrigin = C = typeof D == "string" || D.crossOrigin == null ? void 0 : D.crossOrigin === "use-credentials" ? "use-credentials" : ""), D = l, Z = E, D.scriptResources[Z] = null, D.moduleScriptResources[Z] = null, D = [], In(D, m), x.bootstrapScripts.add(D), U.push(
          $t,
          le(Fe(E)),
          xn
        ), s && U.push(
          Xu,
          le(Fe(s)),
          xn
        ), typeof _ == "string" && U.push(
          et,
          le(Fe(_)),
          xn
        ), typeof C == "string" && U.push(
          hu,
          le(Fe(C)),
          xn
        ), Vi(U, l), U.push(gu);
    if (ce !== void 0)
      for (a = 0; a < ce.length; a++)
        _ = ce[a], E = v = void 0, C = {
          rel: "modulepreload",
          fetchPriority: "low",
          nonce: s
        }, typeof _ == "string" ? C.href = se = _ : (C.href = se = _.src, C.integrity = E = typeof _.integrity == "string" ? _.integrity : void 0, C.crossOrigin = v = typeof _ == "string" || _.crossOrigin == null ? void 0 : _.crossOrigin === "use-credentials" ? "use-credentials" : ""), _ = l, m = se, _.scriptResources[m] = null, _.moduleScriptResources[m] = null, _ = [], In(_, C), x.bootstrapScripts.add(_), U.push(
          Rn,
          le(Fe(se)),
          xn
        ), s && U.push(
          Xu,
          le(Fe(s)),
          xn
        ), typeof E == "string" && U.push(
          et,
          le(Fe(E)),
          xn
        ), typeof v == "string" && U.push(
          hu,
          le(Fe(v)),
          xn
        ), Vi(U, l), U.push(gu);
    return x;
  }
  function Wo(l, a, s, v, x) {
    return {
      idPrefix: l === void 0 ? "" : l,
      nextFormID: 0,
      streamingFormat: 0,
      bootstrapScriptContent: s,
      bootstrapScripts: v,
      bootstrapModules: x,
      instructions: 0,
      hasBody: !1,
      hasHtml: !1,
      unknownResources: {},
      dnsResources: {},
      connectResources: { default: {}, anonymous: {}, credentials: {} },
      imageResources: {},
      styleResources: {},
      scriptResources: {},
      moduleUnknownResources: {},
      moduleScriptResources: {}
    };
  }
  function Nn() {
    return { htmlChunks: null, headChunks: null, bodyChunks: null };
  }
  function An(l, a, s, v) {
    return {
      insertionMode: l,
      selectedValue: a,
      tagScope: s,
      viewTransition: v
    };
  }
  function Fc(l) {
    return An(
      l === "http://www.w3.org/2000/svg" ? 4 : l === "http://www.w3.org/1998/Math/MathML" ? 5 : 0,
      null,
      0,
      null
    );
  }
  function va(l, a, s) {
    var v = l.tagScope & -25;
    switch (a) {
      case "noscript":
        return An(2, null, v | 1, null);
      case "select":
        return An(
          2,
          s.value != null ? s.value : s.defaultValue,
          v,
          null
        );
      case "svg":
        return An(4, null, v, null);
      case "picture":
        return An(2, null, v | 2, null);
      case "math":
        return An(5, null, v, null);
      case "foreignObject":
        return An(2, null, v, null);
      case "table":
        return An(6, null, v, null);
      case "thead":
      case "tbody":
      case "tfoot":
        return An(7, null, v, null);
      case "colgroup":
        return An(9, null, v, null);
      case "tr":
        return An(8, null, v, null);
      case "head":
        if (2 > l.insertionMode)
          return An(3, null, v, null);
        break;
      case "html":
        if (l.insertionMode === 0)
          return An(1, null, v, null);
    }
    return 6 <= l.insertionMode || 2 > l.insertionMode ? An(2, null, v, null) : l.tagScope !== v ? An(
      l.insertionMode,
      l.selectedValue,
      v,
      null
    ) : l;
  }
  function Zu(l) {
    return l === null ? null : {
      update: l.update,
      enter: "none",
      exit: "none",
      share: l.update,
      name: l.autoName,
      autoName: l.autoName,
      nameIdx: 0
    };
  }
  function bu(l, a) {
    return a.tagScope & 32 && (l.instructions |= 128), An(
      a.insertionMode,
      a.selectedValue,
      a.tagScope | 12,
      Zu(a.viewTransition)
    );
  }
  function ll(l, a) {
    l = Zu(a.viewTransition);
    var s = a.tagScope | 16;
    return l !== null && l.share !== "none" && (s |= 64), An(
      a.insertionMode,
      a.selectedValue,
      s,
      l
    );
  }
  var Hl = F("<!-- -->");
  function Ju(l, a, s, v) {
    return a === "" ? v : (v && l.push(Hl), l.push(le(Fe(a))), !0);
  }
  var ba = /* @__PURE__ */ new Map(), qn = F(' style="'), Ni = F(":"), Wi = F(";");
  function ya(l, a) {
    if (typeof a != "object") throw Error(G(62));
    var s = !0, v;
    for (v in a)
      if (He.call(a, v)) {
        var x = a[v];
        if (x != null && typeof x != "boolean" && x !== "") {
          if (v.indexOf("--") === 0) {
            var E = le(Fe(v));
            x = le(
              Fe(("" + x).trim())
            );
          } else
            E = ba.get(v), E === void 0 && (E = F(
              Fe(
                v.replace(lt, "-$1").toLowerCase().replace(un, "-ms-")
              )
            ), ba.set(v, E)), x = typeof x == "number" ? x === 0 || Mt.has(v) ? le("" + x) : le(x + "px") : le(
              Fe(("" + x).trim())
            );
          s ? (s = !1, l.push(
            qn,
            E,
            Ni,
            x
          )) : l.push(Wi, E, Ni, x);
        }
      }
    s || l.push(xn);
  }
  var Ht = F(" "), Or = F('="'), xn = F('"'), Ho = F('=""');
  function Jr(l, a, s) {
    s && typeof s != "function" && typeof s != "symbol" && l.push(Ht, le(a), Ho);
  }
  function je(l, a, s) {
    typeof s != "function" && typeof s != "symbol" && typeof s != "boolean" && l.push(
      Ht,
      le(a),
      Or,
      le(Fe(s)),
      xn
    );
  }
  var Vu = F(
    Fe(
      "javascript:throw new Error('React form unexpectedly submitted.')"
    )
  ), Ye = F('<input type="hidden"');
  function Uo(l, a) {
    this.push(Ye), Yo(l), je(this, "name", a), je(this, "value", l), this.push(Cl);
  }
  function Yo(l) {
    if (typeof l != "string") throw Error(G(480));
  }
  function Go(l, a) {
    if (typeof a.$$FORM_ACTION == "function") {
      var s = l.nextFormID++;
      l = l.idPrefix + s;
      try {
        var v = a.$$FORM_ACTION(l);
        if (v) {
          var x = v.data;
          x != null && x.forEach(Yo);
        }
        return v;
      } catch (E) {
        if (typeof E == "object" && E !== null && typeof E.then == "function")
          throw E;
      }
    }
    return null;
  }
  function Ul(l, a, s, v, x, E, C, _) {
    var m = null;
    if (typeof v == "function") {
      var D = Go(a, v);
      D !== null ? (_ = D.name, v = D.action || "", x = D.encType, E = D.method, C = D.target, m = D.data) : (l.push(
        Ht,
        le("formAction"),
        Or,
        Vu,
        xn
      ), C = E = x = v = _ = null, Qu(a, s));
    }
    return _ != null && hn(l, "name", _), v != null && hn(l, "formAction", v), x != null && hn(l, "formEncType", x), E != null && hn(l, "formMethod", E), C != null && hn(l, "formTarget", C), m;
  }
  function hn(l, a, s) {
    switch (a) {
      case "className":
        je(l, "class", s);
        break;
      case "tabIndex":
        je(l, "tabindex", s);
        break;
      case "dir":
      case "role":
      case "viewBox":
      case "width":
      case "height":
        je(l, a, s);
        break;
      case "style":
        ya(l, s);
        break;
      case "src":
      case "href":
        if (s === "") break;
      case "action":
      case "formAction":
        if (s == null || typeof s == "function" || typeof s == "symbol" || typeof s == "boolean")
          break;
        s = da("" + s), l.push(
          Ht,
          le(a),
          Or,
          le(Fe(s)),
          xn
        );
        break;
      case "defaultValue":
      case "defaultChecked":
      case "innerHTML":
      case "suppressContentEditableWarning":
      case "suppressHydrationWarning":
      case "ref":
        break;
      case "autoFocus":
      case "multiple":
      case "muted":
        Jr(l, a.toLowerCase(), s);
        break;
      case "xlinkHref":
        if (typeof s == "function" || typeof s == "symbol" || typeof s == "boolean")
          break;
        s = da("" + s), l.push(
          Ht,
          le("xlink:href"),
          Or,
          le(Fe(s)),
          xn
        );
        break;
      case "contentEditable":
      case "spellCheck":
      case "draggable":
      case "value":
      case "autoReverse":
      case "externalResourcesRequired":
      case "focusable":
      case "preserveAlpha":
        typeof s != "function" && typeof s != "symbol" && l.push(
          Ht,
          le(a),
          Or,
          le(Fe(s)),
          xn
        );
        break;
      case "inert":
      case "allowFullScreen":
      case "async":
      case "autoPlay":
      case "controls":
      case "default":
      case "defer":
      case "disabled":
      case "disablePictureInPicture":
      case "disableRemotePlayback":
      case "formNoValidate":
      case "hidden":
      case "loop":
      case "noModule":
      case "noValidate":
      case "open":
      case "playsInline":
      case "readOnly":
      case "required":
      case "reversed":
      case "scoped":
      case "seamless":
      case "itemScope":
        s && typeof s != "function" && typeof s != "symbol" && l.push(
          Ht,
          le(a),
          Ho
        );
        break;
      case "capture":
      case "download":
        s === !0 ? l.push(
          Ht,
          le(a),
          Ho
        ) : s !== !1 && typeof s != "function" && typeof s != "symbol" && l.push(
          Ht,
          le(a),
          Or,
          le(Fe(s)),
          xn
        );
        break;
      case "cols":
      case "rows":
      case "size":
      case "span":
        typeof s != "function" && typeof s != "symbol" && !isNaN(s) && 1 <= s && l.push(
          Ht,
          le(a),
          Or,
          le(Fe(s)),
          xn
        );
        break;
      case "rowSpan":
      case "start":
        typeof s == "function" || typeof s == "symbol" || isNaN(s) || l.push(
          Ht,
          le(a),
          Or,
          le(Fe(s)),
          xn
        );
        break;
      case "xlinkActuate":
        je(l, "xlink:actuate", s);
        break;
      case "xlinkArcrole":
        je(l, "xlink:arcrole", s);
        break;
      case "xlinkRole":
        je(l, "xlink:role", s);
        break;
      case "xlinkShow":
        je(l, "xlink:show", s);
        break;
      case "xlinkTitle":
        je(l, "xlink:title", s);
        break;
      case "xlinkType":
        je(l, "xlink:type", s);
        break;
      case "xmlBase":
        je(l, "xml:base", s);
        break;
      case "xmlLang":
        je(l, "xml:lang", s);
        break;
      case "xmlSpace":
        je(l, "xml:space", s);
        break;
      default:
        if ((!(2 < a.length) || a[0] !== "o" && a[0] !== "O" || a[1] !== "n" && a[1] !== "N") && (a = rt.get(a) || a, Mr(a))) {
          switch (typeof s) {
            case "function":
            case "symbol":
              return;
            case "boolean":
              var v = a.toLowerCase().slice(0, 5);
              if (v !== "data-" && v !== "aria-") return;
          }
          l.push(
            Ht,
            le(a),
            Or,
            le(Fe(s)),
            xn
          );
        }
    }
  }
  var $e = F(">"), Cl = F("/>");
  function Ut(l, a, s) {
    if (a != null) {
      if (s != null) throw Error(G(60));
      if (typeof a != "object" || !("__html" in a))
        throw Error(G(61));
      a = a.__html, a != null && l.push(le("" + a));
    }
  }
  function xa(l) {
    var a = "";
    return Te.Children.forEach(l, function(s) {
      s != null && (a += s);
    }), a;
  }
  var mc = F(' selected=""'), er = F(
    `addEventListener("submit",function(a){if(!a.defaultPrevented){var c=a.target,d=a.submitter,e=c.action,b=d;if(d){var f=d.getAttribute("formAction");null!=f&&(e=f,b=null)}"javascript:throw new Error('React form unexpectedly submitted.')"===e&&(a.preventDefault(),b?(a=document.createElement("input"),a.name=b.name,a.value=b.value,b.parentNode.insertBefore(a,b),b=new FormData(c),a.parentNode.removeChild(a)):b=new FormData(c),a=c.ownerDocument||c,(a.$$reactFormReplay=a.$$reactFormReplay||[]).push(c,d,b))}});`
  );
  function Qu(l, a) {
    if (!(l.instructions & 16)) {
      l.instructions |= 16;
      var s = a.preamble, v = a.bootstrapChunks;
      (s.htmlChunks || s.headChunks) && v.length === 0 ? (v.push(a.startInlineScript), Vi(v, l), v.push(
        $e,
        er,
        qe
      )) : v.unshift(
        a.startInlineScript,
        $e,
        er,
        qe
      );
    }
  }
  var yu = F("<!--F!-->"), Ac = F("<!--F-->");
  function In(l, a) {
    l.push(xt("link"));
    for (var s in a)
      if (He.call(a, s)) {
        var v = a[s];
        if (v != null)
          switch (s) {
            case "children":
            case "dangerouslySetInnerHTML":
              throw Error(G(399, "link"));
            default:
              hn(l, s, v);
          }
      }
    return l.push(Cl), null;
  }
  var Ic = /(<\/|<)(s)(tyle)/gi;
  function to(l, a, s, v) {
    return "" + a + (s === "s" ? "\\73 " : "\\53 ") + v;
  }
  function Vr(l, a, s) {
    l.push(xt(s));
    for (var v in a)
      if (He.call(a, v)) {
        var x = a[v];
        if (x != null)
          switch (v) {
            case "children":
            case "dangerouslySetInnerHTML":
              throw Error(G(399, s));
            default:
              hn(l, v, x);
          }
      }
    return l.push(Cl), null;
  }
  function Ta(l, a) {
    l.push(xt("title"));
    var s = null, v = null, x;
    for (x in a)
      if (He.call(a, x)) {
        var E = a[x];
        if (E != null)
          switch (x) {
            case "children":
              s = E;
              break;
            case "dangerouslySetInnerHTML":
              v = E;
              break;
            default:
              hn(l, x, E);
          }
      }
    return l.push($e), a = Array.isArray(s) ? 2 > s.length ? s[0] : null : s, typeof a != "function" && typeof a != "symbol" && a !== null && a !== void 0 && l.push(le(Fe("" + a))), Ut(l, v, s), l.push(Xl("title")), null;
  }
  var Ku = F("<!--head-->"), qu = F("<!--body-->"), ro = F("<!--html-->");
  function lo(l, a) {
    l.push(xt("script"));
    var s = null, v = null, x;
    for (x in a)
      if (He.call(a, x)) {
        var E = a[x];
        if (E != null)
          switch (x) {
            case "children":
              s = E;
              break;
            case "dangerouslySetInnerHTML":
              v = E;
              break;
            default:
              hn(l, x, E);
          }
      }
    return l.push($e), Ut(l, v, s), typeof s == "string" && l.push(
      le(("" + s).replace(vu, No))
    ), l.push(Xl("script")), null;
  }
  function Yl(l, a, s) {
    l.push(xt(s));
    var v = s = null, x;
    for (x in a)
      if (He.call(a, x)) {
        var E = a[x];
        if (E != null)
          switch (x) {
            case "children":
              s = E;
              break;
            case "dangerouslySetInnerHTML":
              v = E;
              break;
            default:
              hn(l, x, E);
          }
      }
    return l.push($e), Ut(l, v, s), s;
  }
  function Gl(l, a, s) {
    l.push(xt(s));
    var v = s = null, x;
    for (x in a)
      if (He.call(a, x)) {
        var E = a[x];
        if (E != null)
          switch (x) {
            case "children":
              s = E;
              break;
            case "dangerouslySetInnerHTML":
              v = E;
              break;
            default:
              hn(l, x, E);
          }
      }
    return l.push($e), Ut(l, v, s), typeof s == "string" ? (l.push(le(Fe(s))), null) : s;
  }
  var Mc = F(`
`), Xo = /^[a-zA-Z][a-zA-Z:_\.\-\d]*$/, Zo = /* @__PURE__ */ new Map();
  function xt(l) {
    var a = Zo.get(l);
    if (a === void 0) {
      if (!Xo.test(l))
        throw Error(G(65, l));
      a = F("<" + l), Zo.set(l, a);
    }
    return a;
  }
  var io = F("<!DOCTYPE html>");
  function Jo(l, a, s, v, x, E, C, _, m) {
    switch (a) {
      case "div":
      case "span":
      case "svg":
      case "path":
        break;
      case "a":
        l.push(xt("a"));
        var D = null, U = null, Z;
        for (Z in s)
          if (He.call(s, Z)) {
            var se = s[Z];
            if (se != null)
              switch (Z) {
                case "children":
                  D = se;
                  break;
                case "dangerouslySetInnerHTML":
                  U = se;
                  break;
                case "href":
                  se === "" ? je(l, "href", "") : hn(l, Z, se);
                  break;
                default:
                  hn(l, Z, se);
              }
          }
        if (l.push($e), Ut(l, U, D), typeof D == "string") {
          l.push(le(Fe(D)));
          var ce = null;
        } else ce = D;
        return ce;
      case "g":
      case "p":
      case "li":
        break;
      case "select":
        l.push(xt("select"));
        var Ge = null, Se = null, Xe;
        for (Xe in s)
          if (He.call(s, Xe)) {
            var Yn = s[Xe];
            if (Yn != null)
              switch (Xe) {
                case "children":
                  Ge = Yn;
                  break;
                case "dangerouslySetInnerHTML":
                  Se = Yn;
                  break;
                case "defaultValue":
                case "value":
                  break;
                default:
                  hn(
                    l,
                    Xe,
                    Yn
                  );
              }
          }
        return l.push($e), Ut(l, Se, Ge), Ge;
      case "option":
        var Fn = _.selectedValue;
        l.push(xt("option"));
        var Zt = null, Dr = null, fl = null, Pe = null, or;
        for (or in s)
          if (He.call(s, or)) {
            var tt = s[or];
            if (tt != null)
              switch (or) {
                case "children":
                  Zt = tt;
                  break;
                case "selected":
                  fl = tt;
                  break;
                case "dangerouslySetInnerHTML":
                  Pe = tt;
                  break;
                case "value":
                  Dr = tt;
                default:
                  hn(
                    l,
                    or,
                    tt
                  );
              }
          }
        if (Fn != null) {
          var kr = Dr !== null ? "" + Dr : xa(Zt);
          if (Ee(Fn)) {
            for (var Pi = 0; Pi < Fn.length; Pi++)
              if ("" + Fn[Pi] === kr) {
                l.push(mc);
                break;
              }
          } else
            "" + Fn === kr && l.push(mc);
        } else fl && l.push(mc);
        return l.push($e), Ut(l, Pe, Zt), Zt;
      case "textarea":
        l.push(xt("textarea"));
        var Gn = null, Na = null, dl = null, cr;
        for (cr in s)
          if (He.call(s, cr)) {
            var _l = s[cr];
            if (_l != null)
              switch (cr) {
                case "children":
                  dl = _l;
                  break;
                case "value":
                  Gn = _l;
                  break;
                case "defaultValue":
                  Na = _l;
                  break;
                case "dangerouslySetInnerHTML":
                  throw Error(G(91));
                default:
                  hn(
                    l,
                    cr,
                    _l
                  );
              }
          }
        if (Gn === null && Na !== null && (Gn = Na), l.push($e), dl != null) {
          if (Gn != null) throw Error(G(92));
          if (Ee(dl)) {
            if (1 < dl.length)
              throw Error(G(93));
            Gn = "" + dl[0];
          }
          Gn = "" + dl;
        }
        return typeof Gn == "string" && Gn[0] === `
` && l.push(Mc), Gn !== null && l.push(
          le(Fe("" + Gn))
        ), null;
      case "input":
        l.push(xt("input"));
        var ur = null, Wa = null, Ro = null, Sr = null, Co = null, sr = null, Ha = null, Du = null, Dn = null, ko;
        for (ko in s)
          if (He.call(s, ko)) {
            var Dl = s[ko];
            if (Dl != null)
              switch (ko) {
                case "children":
                case "dangerouslySetInnerHTML":
                  throw Error(G(399, "input"));
                case "name":
                  ur = Dl;
                  break;
                case "formAction":
                  Wa = Dl;
                  break;
                case "formEncType":
                  Ro = Dl;
                  break;
                case "formMethod":
                  Sr = Dl;
                  break;
                case "formTarget":
                  Co = Dl;
                  break;
                case "defaultChecked":
                  Dn = Dl;
                  break;
                case "defaultValue":
                  Ha = Dl;
                  break;
                case "checked":
                  Du = Dl;
                  break;
                case "value":
                  sr = Dl;
                  break;
                default:
                  hn(
                    l,
                    ko,
                    Dl
                  );
              }
          }
        var Kc = Ul(
          l,
          v,
          x,
          Wa,
          Ro,
          Sr,
          Co,
          ur
        );
        return Du !== null ? Jr(l, "checked", Du) : Dn !== null && Jr(l, "checked", Dn), sr !== null ? hn(l, "value", sr) : Ha !== null && hn(l, "value", Ha), l.push(Cl), Kc != null && Kc.forEach(Uo, l), null;
      case "button":
        l.push(xt("button"));
        var Ua = null, qc = null, jc = null, So = null, aa = null, $c = null, $l = null, Ya;
        for (Ya in s)
          if (He.call(s, Ya)) {
            var Ga = s[Ya];
            if (Ga != null)
              switch (Ya) {
                case "children":
                  Ua = Ga;
                  break;
                case "dangerouslySetInnerHTML":
                  qc = Ga;
                  break;
                case "name":
                  jc = Ga;
                  break;
                case "formAction":
                  So = Ga;
                  break;
                case "formEncType":
                  aa = Ga;
                  break;
                case "formMethod":
                  $c = Ga;
                  break;
                case "formTarget":
                  $l = Ga;
                  break;
                default:
                  hn(
                    l,
                    Ya,
                    Ga
                  );
              }
          }
        var as = Ul(
          l,
          v,
          x,
          So,
          aa,
          $c,
          $l,
          jc
        );
        if (l.push($e), as != null && as.forEach(Uo, l), Ut(l, qc, Ua), typeof Ua == "string") {
          l.push(
            le(Fe(Ua))
          );
          var hl = null;
        } else hl = Ua;
        return hl;
      case "form":
        l.push(xt("form"));
        var os = null, tl = null, oa = null, Fi = null, eu = null, nu = null, tu;
        for (tu in s)
          if (He.call(s, tu)) {
            var Xa = s[tu];
            if (Xa != null)
              switch (tu) {
                case "children":
                  os = Xa;
                  break;
                case "dangerouslySetInnerHTML":
                  tl = Xa;
                  break;
                case "action":
                  oa = Xa;
                  break;
                case "encType":
                  Fi = Xa;
                  break;
                case "method":
                  eu = Xa;
                  break;
                case "target":
                  nu = Xa;
                  break;
                default:
                  hn(
                    l,
                    tu,
                    Xa
                  );
              }
          }
        var cs = null, Lu = null;
        if (typeof oa == "function") {
          var Po = Go(
            v,
            oa
          );
          Po !== null ? (oa = Po.action || "", Fi = Po.encType, eu = Po.method, nu = Po.target, cs = Po.data, Lu = Po.name) : (l.push(
            Ht,
            le("action"),
            Or,
            Vu,
            xn
          ), nu = eu = Fi = oa = null, Qu(v, x));
        }
        if (oa != null && hn(l, "action", oa), Fi != null && hn(l, "encType", Fi), eu != null && hn(l, "method", eu), nu != null && hn(l, "target", nu), l.push($e), Lu !== null && (l.push(Ye), je(l, "name", Lu), l.push(Cl), cs != null && cs.forEach(Uo, l)), Ut(l, tl, os), typeof os == "string") {
          l.push(
            le(Fe(os))
          );
          var us = null;
        } else us = os;
        return us;
      case "menuitem":
        l.push(xt("menuitem"));
        for (var ru in s)
          if (He.call(s, ru)) {
            var xs = s[ru];
            if (xs != null)
              switch (ru) {
                case "children":
                case "dangerouslySetInnerHTML":
                  throw Error(G(400));
                default:
                  hn(
                    l,
                    ru,
                    xs
                  );
              }
          }
        return l.push($e), null;
      case "object":
        l.push(xt("object"));
        var zu = null, Ts = null, lu;
        for (lu in s)
          if (He.call(s, lu)) {
            var Fo = s[lu];
            if (Fo != null)
              switch (lu) {
                case "children":
                  zu = Fo;
                  break;
                case "dangerouslySetInnerHTML":
                  Ts = Fo;
                  break;
                case "data":
                  var Ss = da("" + Fo);
                  if (Ss === "") break;
                  l.push(
                    Ht,
                    le("data"),
                    Or,
                    le(Fe(Ss)),
                    xn
                  );
                  break;
                default:
                  hn(
                    l,
                    lu,
                    Fo
                  );
              }
          }
        if (l.push($e), Ut(l, Ts, zu), typeof zu == "string") {
          l.push(
            le(Fe(zu))
          );
          var Ps = null;
        } else Ps = zu;
        return Ps;
      case "title":
        var ei = _.tagScope & 1, ss = _.tagScope & 4;
        if (_.insertionMode === 4 || ei || s.itemProp != null)
          var Fs = Ta(
            l,
            s
          );
        else
          ss ? Fs = null : (Ta(x.hoistableChunks, s), Fs = void 0);
        return Fs;
      case "link":
        var Lt = _.tagScope & 1, mi = _.tagScope & 4, fr = s.rel, ca = s.href, gl = s.precedence;
        if (_.insertionMode === 4 || Lt || s.itemProp != null || typeof fr != "string" || typeof ca != "string" || ca === "") {
          In(l, s);
          var fn = null;
        } else if (s.rel === "stylesheet")
          if (typeof gl != "string" || s.disabled != null || s.onLoad || s.onError)
            fn = In(
              l,
              s
            );
          else {
            var Lr = x.styles.get(gl), Ai = v.styleResources.hasOwnProperty(ca) ? v.styleResources[ca] : void 0;
            if (Ai !== null) {
              v.styleResources[ca] = null, Lr || (Lr = {
                precedence: le(Fe(gl)),
                rules: [],
                hrefs: [],
                sheets: /* @__PURE__ */ new Map()
              }, x.styles.set(gl, Lr));
              var st = {
                state: 0,
                props: Ze({}, s, {
                  "data-precedence": s.precedence,
                  precedence: null
                })
              };
              if (Ai) {
                Ai.length === 2 && Pa(st.props, Ai);
                var vc = x.preloads.stylesheets.get(ca);
                vc && 0 < vc.length ? vc.length = 0 : st.state = 1;
              }
              Lr.sheets.set(ca, st), C && C.stylesheets.add(st);
            } else if (Lr) {
              var iu = Lr.sheets.get(ca);
              iu && C && C.stylesheets.add(iu);
            }
            m && l.push(Hl), fn = null;
          }
        else
          s.onLoad || s.onError ? fn = In(
            l,
            s
          ) : (m && l.push(Hl), fn = mi ? null : In(x.hoistableChunks, s));
        return fn;
      case "script":
        var fs = _.tagScope & 1, mo = s.async;
        if (typeof s.src != "string" || !s.src || !mo || typeof mo == "function" || typeof mo == "symbol" || s.onLoad || s.onError || _.insertionMode === 4 || fs || s.itemProp != null)
          var Za = lo(
            l,
            s
          );
        else {
          var bc = s.src;
          if (s.type === "module")
            var Ll = v.moduleScriptResources, Ja = x.preloads.moduleScripts;
          else
            Ll = v.scriptResources, Ja = x.preloads.scripts;
          var yc = Ll.hasOwnProperty(bc) ? Ll[bc] : void 0;
          if (yc !== null) {
            Ll[bc] = null;
            var n = s;
            if (yc) {
              yc.length === 2 && (n = Ze({}, s), Pa(n, yc));
              var r = Ja.get(bc);
              r && (r.length = 0);
            }
            var u = [];
            x.scripts.add(u), lo(u, n);
          }
          m && l.push(Hl), Za = null;
        }
        return Za;
      case "style":
        var h = _.tagScope & 1, y = s.precedence, p = s.href, P = s.nonce;
        if (_.insertionMode === 4 || h || s.itemProp != null || typeof y != "string" || typeof p != "string" || p === "") {
          l.push(xt("style"));
          var M = null, K = null, L;
          for (L in s)
            if (He.call(s, L)) {
              var J = s[L];
              if (J != null)
                switch (L) {
                  case "children":
                    M = J;
                    break;
                  case "dangerouslySetInnerHTML":
                    K = J;
                    break;
                  default:
                    hn(
                      l,
                      L,
                      J
                    );
                }
            }
          l.push($e);
          var ie = Array.isArray(M) ? 2 > M.length ? M[0] : null : M;
          typeof ie != "function" && typeof ie != "symbol" && ie !== null && ie !== void 0 && l.push(
            le(("" + ie).replace(Ic, to))
          ), Ut(l, K, M), l.push(Xl("style"));
          var fe = null;
        } else {
          var oe = x.styles.get(y);
          if ((v.styleResources.hasOwnProperty(p) ? v.styleResources[p] : void 0) !== null) {
            v.styleResources[p] = null, oe || (oe = {
              precedence: le(
                Fe(y)
              ),
              rules: [],
              hrefs: [],
              sheets: /* @__PURE__ */ new Map()
            }, x.styles.set(y, oe));
            var $ = x.nonce.style;
            if (!$ || $ === P) {
              oe.hrefs.push(
                le(Fe(p))
              );
              var Ne = oe.rules, Cn = null, Re = null, ln;
              for (ln in s)
                if (He.call(s, ln)) {
                  var Jt = s[ln];
                  if (Jt != null)
                    switch (ln) {
                      case "children":
                        Cn = Jt;
                        break;
                      case "dangerouslySetInnerHTML":
                        Re = Jt;
                    }
                }
              var Vt = Array.isArray(Cn) ? 2 > Cn.length ? Cn[0] : null : Cn;
              typeof Vt != "function" && typeof Vt != "symbol" && Vt !== null && Vt !== void 0 && Ne.push(
                le(
                  ("" + Vt).replace(Ic, to)
                )
              ), Ut(Ne, Re, Cn);
            }
          }
          oe && C && C.styles.add(oe), m && l.push(Hl), fe = void 0;
        }
        return fe;
      case "meta":
        var cn = _.tagScope & 1, mt = _.tagScope & 4;
        if (_.insertionMode === 4 || cn || s.itemProp != null)
          var ni = Vr(
            l,
            s,
            "meta"
          );
        else
          m && l.push(Hl), ni = mt ? null : typeof s.charSet == "string" ? Vr(x.charsetChunks, s, "meta") : s.name === "viewport" ? Vr(x.viewportChunks, s, "meta") : Vr(x.hoistableChunks, s, "meta");
        return ni;
      case "listing":
      case "pre":
        l.push(xt(a));
        var Pr = null, he = null, vn;
        for (vn in s)
          if (He.call(s, vn)) {
            var bn = s[vn];
            if (bn != null)
              switch (vn) {
                case "children":
                  Pr = bn;
                  break;
                case "dangerouslySetInnerHTML":
                  he = bn;
                  break;
                default:
                  hn(
                    l,
                    vn,
                    bn
                  );
              }
          }
        if (l.push($e), he != null) {
          if (Pr != null) throw Error(G(60));
          if (typeof he != "object" || !("__html" in he))
            throw Error(G(61));
          var wn = he.__html;
          wn != null && (typeof wn == "string" && 0 < wn.length && wn[0] === `
` ? l.push(Mc, le(wn)) : l.push(le("" + wn)));
        }
        return typeof Pr == "string" && Pr[0] === `
` && l.push(Mc), Pr;
      case "img":
        var Xn = _.tagScope & 3, Ve = s.src, Oe = s.srcSet;
        if (!(s.loading === "lazy" || !Ve && !Oe || typeof Ve != "string" && Ve != null || typeof Oe != "string" && Oe != null || s.fetchPriority === "low" || Xn) && (typeof Ve != "string" || Ve[4] !== ":" || Ve[0] !== "d" && Ve[0] !== "D" || Ve[1] !== "a" && Ve[1] !== "A" || Ve[2] !== "t" && Ve[2] !== "T" || Ve[3] !== "a" && Ve[3] !== "A") && (typeof Oe != "string" || Oe[4] !== ":" || Oe[0] !== "d" && Oe[0] !== "D" || Oe[1] !== "a" && Oe[1] !== "A" || Oe[2] !== "t" && Oe[2] !== "T" || Oe[3] !== "a" && Oe[3] !== "A")) {
          C !== null && _.tagScope & 64 && (C.suspenseyImages = !0);
          var dr = typeof s.sizes == "string" ? s.sizes : void 0, yn = Oe ? Oe + `
` + (dr || "") : Ve, Qt = x.preloads.images, Ln = Qt.get(yn);
          if (Ln)
            (s.fetchPriority === "high" || 10 > x.highImagePreloads.size) && (Qt.delete(yn), x.highImagePreloads.add(Ln));
          else if (!v.imageResources.hasOwnProperty(yn)) {
            v.imageResources[yn] = yt;
            var zr = s.crossOrigin, Ao = typeof zr == "string" ? zr === "use-credentials" ? zr : "" : void 0, Br = x.headers, ti;
            Br && 0 < Br.remainingCapacity && typeof s.srcSet != "string" && (s.fetchPriority === "high" || 500 > Br.highImagePreloads.length) && (ti = Nc(Ve, "image", {
              imageSrcSet: s.srcSet,
              imageSizes: s.sizes,
              crossOrigin: Ao,
              integrity: s.integrity,
              nonce: s.nonce,
              type: s.type,
              fetchPriority: s.fetchPriority,
              referrerPolicy: s.refererPolicy
            }), 0 <= (Br.remainingCapacity -= ti.length + 2)) ? (x.resets.image[yn] = yt, Br.highImagePreloads && (Br.highImagePreloads += ", "), Br.highImagePreloads += ti) : (Ln = [], In(Ln, {
              rel: "preload",
              as: "image",
              href: Oe ? void 0 : Ve,
              imageSrcSet: Oe,
              imageSizes: dr,
              crossOrigin: Ao,
              integrity: s.integrity,
              type: s.type,
              fetchPriority: s.fetchPriority,
              referrerPolicy: s.referrerPolicy
            }), s.fetchPriority === "high" || 10 > x.highImagePreloads.size ? x.highImagePreloads.add(Ln) : (x.bulkPreloads.add(Ln), Qt.set(yn, Ln)));
          }
        }
        return Vr(l, s, "img");
      case "base":
      case "area":
      case "br":
      case "col":
      case "embed":
      case "hr":
      case "keygen":
      case "param":
      case "source":
      case "track":
      case "wbr":
        return Vr(l, s, a);
      case "annotation-xml":
      case "color-profile":
      case "font-face":
      case "font-face-src":
      case "font-face-uri":
      case "font-face-format":
      case "font-face-name":
      case "missing-glyph":
        break;
      case "head":
        if (2 > _.insertionMode) {
          var ri = E || x.preamble;
          if (ri.headChunks)
            throw Error(G(545, "`<head>`"));
          E !== null && l.push(Ku), ri.headChunks = [];
          var li = Yl(
            ri.headChunks,
            s,
            "head"
          );
        } else
          li = Gl(
            l,
            s,
            "head"
          );
        return li;
      case "body":
        if (2 > _.insertionMode) {
          var zt = E || x.preamble;
          if (zt.bodyChunks)
            throw Error(G(545, "`<body>`"));
          E !== null && l.push(qu), zt.bodyChunks = [];
          var e = Yl(
            zt.bodyChunks,
            s,
            "body"
          );
        } else
          e = Gl(
            l,
            s,
            "body"
          );
        return e;
      case "html":
        if (_.insertionMode === 0) {
          var t = E || x.preamble;
          if (t.htmlChunks)
            throw Error(G(545, "`<html>`"));
          E !== null && l.push(ro), t.htmlChunks = [io];
          var c = Yl(
            t.htmlChunks,
            s,
            "html"
          );
        } else
          c = Gl(
            l,
            s,
            "html"
          );
        return c;
      default:
        if (a.indexOf("-") !== -1) {
          l.push(xt(a));
          var d = null, b = null, w;
          for (w in s)
            if (He.call(s, w)) {
              var k = s[w];
              if (k != null) {
                var A = w;
                switch (w) {
                  case "children":
                    d = k;
                    break;
                  case "dangerouslySetInnerHTML":
                    b = k;
                    break;
                  case "style":
                    ya(l, k);
                    break;
                  case "suppressContentEditableWarning":
                  case "suppressHydrationWarning":
                  case "ref":
                    break;
                  case "className":
                    A = "class";
                  default:
                    if (Mr(w) && typeof k != "function" && typeof k != "symbol" && k !== !1) {
                      if (k === !0) k = "";
                      else if (typeof k == "object") continue;
                      l.push(
                        Ht,
                        le(A),
                        Or,
                        le(Fe(k)),
                        xn
                      );
                    }
                }
              }
            }
          return l.push($e), Ut(l, b, d), d;
        }
    }
    return Gl(l, s, a);
  }
  var Oc = /* @__PURE__ */ new Map();
  function Xl(l) {
    var a = Oc.get(l);
    return a === void 0 && (a = F("</" + l + ">"), Oc.set(l, a)), a;
  }
  function Vo(l, a) {
    l = l.preamble, l.htmlChunks === null && a.htmlChunks && (l.htmlChunks = a.htmlChunks), l.headChunks === null && a.headChunks && (l.headChunks = a.headChunks), l.bodyChunks === null && a.bodyChunks && (l.bodyChunks = a.bodyChunks);
  }
  function Tr(l, a) {
    a = a.bootstrapChunks;
    for (var s = 0; s < a.length - 1; s++)
      V(l, a[s]);
    return s < a.length ? (s = a[s], a.length = 0, ue(l, s)) : !0;
  }
  var kl = F(
    "requestAnimationFrame(function(){$RT=performance.now()});"
  ), xu = F('<template id="'), Qo = F('"></template>'), wr = F("<!--&-->"), Ko = F("<!--/&-->"), Hi = F("<!--$-->"), ao = F(
    '<!--$?--><template id="'
  ), pr = F('"></template>'), St = F("<!--$!-->"), Sn = F("<!--/$-->"), Tu = F("<template"), fi = F('"'), Sl = F(' data-dgst="');
  F(' data-msg="'), F(' data-stck="'), F(' data-cstck="');
  var wu = F("></template>");
  function di(l, a, s) {
    if (V(l, ao), s === null) throw Error(G(395));
    return V(l, a.boundaryPrefix), V(l, le(s.toString(16))), ue(l, pr);
  }
  var Zl = F('<div hidden id="'), oo = F('">'), hi = F("</div>"), gi = F(
    '<svg aria-hidden="true" style="display:none" id="'
  ), co = F('">'), Ie = F("</svg>"), Pl = F(
    '<math aria-hidden="true" style="display:none" id="'
  ), it = F('">'), Jl = F("</math>"), Tt = F('<table hidden id="'), Ui = F('">'), wa = F("</table>"), Vl = F('<table hidden><tbody id="'), vi = F('">'), at = F("</tbody></table>"), bi = F('<table hidden><tr id="'), yi = F('">'), pa = F("</tr></table>"), Yi = F(
    '<table hidden><colgroup id="'
  ), Ea = F('">'), Er = F("</colgroup></table>");
  function Ra(l, a, s, v) {
    switch (s.insertionMode) {
      case 0:
      case 1:
      case 3:
      case 2:
        return V(l, Zl), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, oo);
      case 4:
        return V(l, gi), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, co);
      case 5:
        return V(l, Pl), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, it);
      case 6:
        return V(l, Tt), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, Ui);
      case 7:
        return V(l, Vl), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, vi);
      case 8:
        return V(l, bi), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, yi);
      case 9:
        return V(l, Yi), V(l, a.segmentPrefix), V(l, le(v.toString(16))), ue(l, Ea);
      default:
        throw Error(G(397));
    }
  }
  function Fl(l, a) {
    switch (a.insertionMode) {
      case 0:
      case 1:
      case 3:
      case 2:
        return ue(l, hi);
      case 4:
        return ue(l, Ie);
      case 5:
        return ue(l, Jl);
      case 6:
        return ue(l, wa);
      case 7:
        return ue(l, at);
      case 8:
        return ue(l, pa);
      case 9:
        return ue(l, Er);
      default:
        throw Error(G(397));
    }
  }
  var pu = F(
    '$RS=function(a,b){a=document.getElementById(a);b=document.getElementById(b);for(a.parentNode.removeChild(a);a.firstChild;)b.parentNode.insertBefore(a.firstChild,b);b.parentNode.removeChild(b)};$RS("'
  ), Eu = F('$RS("'), Ru = F('","'), uo = F('")<\/script>');
  F('<template data-rsi="" data-sid="'), F('" data-pid="');
  var so = F(
    `$RB=[];$RV=function(a){$RT=performance.now();for(var b=0;b<a.length;b+=2){var c=a[b],e=a[b+1];null!==e.parentNode&&e.parentNode.removeChild(e);var f=c.parentNode;if(f){var g=c.previousSibling,h=0;do{if(c&&8===c.nodeType){var d=c.data;if("/$"===d||"/&"===d)if(0===h)break;else h--;else"$"!==d&&"$?"!==d&&"$~"!==d&&"$!"!==d&&"&"!==d||h++}d=c.nextSibling;f.removeChild(c);c=d}while(c);for(;e.firstChild;)f.insertBefore(e.firstChild,c);g.data="$";g._reactRetry&&requestAnimationFrame(g._reactRetry)}}a.length=0};
$RC=function(a,b){if(b=document.getElementById(b))(a=document.getElementById(a))?(a.previousSibling.data="$~",$RB.push(a,b),2===$RB.length&&("number"!==typeof $RT?requestAnimationFrame($RV.bind(null,$RB)):(a=performance.now(),setTimeout($RV.bind(null,$RB),2300>a&&2E3<a?2300-a:$RT+300-a)))):b.parentNode.removeChild(b)};`
  );
  le(
    `$RV=function(A,g){function k(a,b){var e=a.getAttribute(b);e&&(b=a.style,l.push(a,b.viewTransitionName,b.viewTransitionClass),"auto"!==e&&(b.viewTransitionClass=e),(a=a.getAttribute("vt-name"))||(a="_T_"+K++ +"_"),b.viewTransitionName=a,B=!0)}var B=!1,K=0,l=[];try{var f=document.__reactViewTransition;if(f){f.finished.finally($RV.bind(null,g));return}var m=new Map;for(f=1;f<g.length;f+=2)for(var h=g[f].querySelectorAll("[vt-share]"),d=0;d<h.length;d++){var c=h[d];m.set(c.getAttribute("vt-name"),c)}var u=[];for(h=0;h<g.length;h+=2){var C=g[h],x=C.parentNode;if(x){var v=x.getBoundingClientRect();if(v.left||v.top||v.width||v.height){c=C;for(f=0;c;){if(8===c.nodeType){var r=c.data;if("/$"===r)if(0===f)break;else f--;else"$"!==r&&"$?"!==r&&"$~"!==r&&"$!"!==r||f++}else if(1===c.nodeType){d=c;var D=d.getAttribute("vt-name"),y=m.get(D);k(d,y?"vt-share":"vt-exit");y&&(k(y,"vt-share"),m.set(D,null));var E=d.querySelectorAll("[vt-share]");for(d=0;d<E.length;d++){var F=E[d],G=F.getAttribute("vt-name"),
H=m.get(G);H&&(k(F,"vt-share"),k(H,"vt-share"),m.set(G,null))}}c=c.nextSibling}for(var I=g[h+1],t=I.firstElementChild;t;)null!==m.get(t.getAttribute("vt-name"))&&k(t,"vt-enter"),t=t.nextElementSibling;c=x;do for(var n=c.firstElementChild;n;){var J=n.getAttribute("vt-update");J&&"none"!==J&&!l.includes(n)&&k(n,"vt-update");n=n.nextElementSibling}while((c=c.parentNode)&&1===c.nodeType&&"none"!==c.getAttribute("vt-update"));u.push.apply(u,I.querySelectorAll('img[src]:not([loading="lazy"])'))}}}if(B){var z=
document.__reactViewTransition=document.startViewTransition({update:function(){A(g);for(var a=[document.documentElement.clientHeight,document.fonts.ready],b={},e=0;e<u.length;b={g:b.g},e++)if(b.g=u[e],!b.g.complete){var p=b.g.getBoundingClientRect();0<p.bottom&&0<p.right&&p.top<window.innerHeight&&p.left<window.innerWidth&&(p=new Promise(function(w){return function(q){w.g.addEventListener("load",q);w.g.addEventListener("error",q)}}(b)),a.push(p))}return Promise.race([Promise.all(a),new Promise(function(w){var q=
performance.now();setTimeout(w,2300>q&&2E3<q?2300-q:500)})])},types:[]});z.ready.finally(function(){for(var a=l.length-3;0<=a;a-=3){var b=l[a],e=b.style;e.viewTransitionName=l[a+1];e.viewTransitionClass=l[a+1];""===b.getAttribute("style")&&b.removeAttribute("style")}});z.finished.finally(function(){document.__reactViewTransition===z&&(document.__reactViewTransition=null)});$RB=[];return}}catch(a){}A(g)}.bind(null,$RV);`
  );
  var fo = F('$RC("'), Pt = F(
    `$RM=new Map;$RR=function(n,w,p){function u(q){this._p=null;q()}for(var r=new Map,t=document,h,b,e=t.querySelectorAll("link[data-precedence],style[data-precedence]"),v=[],k=0;b=e[k++];)"not all"===b.getAttribute("media")?v.push(b):("LINK"===b.tagName&&$RM.set(b.getAttribute("href"),b),r.set(b.dataset.precedence,h=b));e=0;b=[];var l,a;for(k=!0;;){if(k){var f=p[e++];if(!f){k=!1;e=0;continue}var c=!1,m=0;var d=f[m++];if(a=$RM.get(d)){var g=a._p;c=!0}else{a=t.createElement("link");a.href=d;a.rel=
"stylesheet";for(a.dataset.precedence=l=f[m++];g=f[m++];)a.setAttribute(g,f[m++]);g=a._p=new Promise(function(q,x){a.onload=u.bind(a,q);a.onerror=u.bind(a,x)});$RM.set(d,a)}d=a.getAttribute("media");!g||d&&!matchMedia(d).matches||b.push(g);if(c)continue}else{a=v[e++];if(!a)break;l=a.getAttribute("data-precedence");a.removeAttribute("media")}c=r.get(l)||h;c===h&&(h=a);r.set(l,a);c?c.parentNode.insertBefore(a,c.nextSibling):(c=t.head,c.insertBefore(a,c.firstChild))}if(p=document.getElementById(n))p.previousSibling.data=
"$~";Promise.all(b).then($RC.bind(null,n,w),$RX.bind(null,n,"CSS failed to load"))};$RR("`
  ), ju = F('$RR("'), Gi = F('","'), qo = F('",'), _c = F('"'), xi = F(")<\/script>");
  F('<template data-rci="" data-bid="'), F('<template data-rri="" data-bid="'), F('" data-sid="'), F('" data-sty="');
  var Xi = F(
    '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};'
  ), ho = F(
    '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};;$RX("'
  ), ml = F('$RX("'), Zi = F('"'), Ji = F(","), Cu = F(")<\/script>");
  F('<template data-rxi="" data-bid="'), F('" data-dgst="'), F('" data-msg="'), F('" data-stck="'), F('" data-cstck="');
  var vs = /[<\u2028\u2029]/g;
  function Ca(l) {
    return JSON.stringify(l).replace(
      vs,
      function(a) {
        switch (a) {
          case "<":
            return "\\u003c";
          case "\u2028":
            return "\\u2028";
          case "\u2029":
            return "\\u2029";
          default:
            throw Error(
              "escapeJSStringsForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
            );
        }
      }
    );
  }
  var ot = /[&><\u2028\u2029]/g;
  function Ql(l) {
    return JSON.stringify(l).replace(
      ot,
      function(a) {
        switch (a) {
          case "&":
            return "\\u0026";
          case ">":
            return "\\u003e";
          case "<":
            return "\\u003c";
          case "\u2028":
            return "\\u2028";
          case "\u2029":
            return "\\u2029";
          default:
            throw Error(
              "escapeJSObjectForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
            );
        }
      }
    );
  }
  var jo = F(
    ' media="not all" data-precedence="'
  ), $o = F('" data-href="'), Dc = F('">'), Qr = F("</style>"), Kr = !1, _r = !0;
  function qr(l) {
    var a = l.rules, s = l.hrefs, v = 0;
    if (s.length) {
      for (V(this, Rl.startInlineStyle), V(this, jo), V(this, l.precedence), V(this, $o); v < s.length - 1; v++)
        V(this, s[v]), V(this, ec);
      for (V(this, s[v]), V(this, Dc), v = 0; v < a.length; v++) V(this, a[v]);
      _r = ue(
        this,
        Qr
      ), Kr = !0, a.length = 0, s.length = 0;
    }
  }
  function Tn(l) {
    return l.state !== 2 ? Kr = !0 : !1;
  }
  function jr(l, a, s) {
    return Kr = !1, _r = !0, Rl = s, a.styles.forEach(qr, l), Rl = null, a.stylesheets.forEach(Tn), Kr && (s.stylesToHoist = !0), _r;
  }
  function Mn(l) {
    for (var a = 0; a < l.length; a++) V(this, l[a]);
    l.length = 0;
  }
  var Al = [];
  function Lc(l) {
    In(Al, l.props);
    for (var a = 0; a < Al.length; a++)
      V(this, Al[a]);
    Al.length = 0, l.state = 2;
  }
  var zc = F(' data-precedence="'), Bc = F('" data-href="'), ec = F(" "), Ti = F('">'), nc = F("</style>");
  function tc(l) {
    var a = 0 < l.sheets.size;
    l.sheets.forEach(Lc, this), l.sheets.clear();
    var s = l.rules, v = l.hrefs;
    if (!a || v.length) {
      if (V(this, Rl.startInlineStyle), V(this, zc), V(this, l.precedence), l = 0, v.length) {
        for (V(this, Bc); l < v.length - 1; l++)
          V(this, v[l]), V(this, ec);
        V(this, v[l]);
      }
      for (V(this, Ti), l = 0; l < s.length; l++)
        V(this, s[l]);
      V(this, nc), s.length = 0, v.length = 0;
    }
  }
  function Rr(l) {
    if (l.state === 0) {
      l.state = 1;
      var a = l.props;
      for (In(Al, {
        rel: "preload",
        as: "style",
        href: l.props.href,
        crossOrigin: a.crossOrigin,
        fetchPriority: a.fetchPriority,
        integrity: a.integrity,
        media: a.media,
        hrefLang: a.hrefLang,
        referrerPolicy: a.referrerPolicy
      }), l = 0; l < Al.length; l++)
        V(this, Al[l]);
      Al.length = 0;
    }
  }
  function On(l) {
    l.sheets.forEach(Rr, this), l.sheets.clear();
  }
  F('<link rel="expect" href="#'), F('" blocking="render"/>');
  var en = F(' id="');
  function Vi(l, a) {
    !(a.instructions & 32) && (a.instructions |= 32, l.push(
      en,
      le(Fe("_" + a.idPrefix + "R_")),
      xn
    ));
  }
  var ka = F("["), Sa = F(",["), Qi = F(","), ku = F("]");
  function _t(l, a) {
    V(l, ka);
    var s = ka;
    a.stylesheets.forEach(function(v) {
      if (v.state !== 2)
        if (v.state === 3)
          V(l, s), V(
            l,
            le(
              Ql("" + v.props.href)
            )
          ), V(l, ku), s = Sa;
        else {
          V(l, s);
          var x = v.props["data-precedence"], E = v.props, C = da("" + v.props.href);
          V(
            l,
            le(Ql(C))
          ), x = "" + x, V(l, Qi), V(
            l,
            le(Ql(x))
          );
          for (var _ in E)
            if (He.call(E, _) && (x = E[_], x != null))
              switch (_) {
                case "href":
                case "rel":
                case "precedence":
                case "data-precedence":
                  break;
                case "children":
                case "dangerouslySetInnerHTML":
                  throw Error(G(399, "link"));
                default:
                  bs(
                    l,
                    _,
                    x
                  );
              }
          V(l, ku), s = Sa, v.state = 3;
        }
    }), V(l, ku);
  }
  function bs(l, a, s) {
    var v = a.toLowerCase();
    switch (typeof s) {
      case "function":
      case "symbol":
        return;
    }
    switch (a) {
      case "innerHTML":
      case "dangerouslySetInnerHTML":
      case "suppressContentEditableWarning":
      case "suppressHydrationWarning":
      case "style":
      case "ref":
        return;
      case "className":
        v = "class", a = "" + s;
        break;
      case "hidden":
        if (s === !1) return;
        a = "";
        break;
      case "src":
      case "href":
        s = da(s), a = "" + s;
        break;
      default:
        if (2 < a.length && (a[0] === "o" || a[0] === "O") && (a[1] === "n" || a[1] === "N") || !Mr(a))
          return;
        a = "" + s;
    }
    V(l, Qi), V(
      l,
      le(Ql(v))
    ), V(l, Qi), V(
      l,
      le(Ql(a))
    );
  }
  function Il() {
    return { styles: /* @__PURE__ */ new Set(), stylesheets: /* @__PURE__ */ new Set(), suspenseyImages: !1 };
  }
  function $u(l) {
    var a = ze || null;
    if (a) {
      var s = a.resumableState, v = a.renderState;
      if (typeof l == "string" && l) {
        if (!s.dnsResources.hasOwnProperty(l)) {
          s.dnsResources[l] = null, s = v.headers;
          var x, E;
          (E = s && 0 < s.remainingCapacity) && (E = (x = "<" + ("" + l).replace(
            Fa,
            Gt
          ) + ">; rel=dns-prefetch", 0 <= (s.remainingCapacity -= x.length + 2))), E ? (v.resets.dns[l] = null, s.preconnects && (s.preconnects += ", "), s.preconnects += x) : (x = [], In(x, { href: l, rel: "dns-prefetch" }), v.preconnects.add(x));
        }
        ia(a);
      }
    } else El.D(l);
  }
  function Su(l, a) {
    var s = ze || null;
    if (s) {
      var v = s.resumableState, x = s.renderState;
      if (typeof l == "string" && l) {
        var E = a === "use-credentials" ? "credentials" : typeof a == "string" ? "anonymous" : "default";
        if (!v.connectResources[E].hasOwnProperty(l)) {
          v.connectResources[E][l] = null, v = x.headers;
          var C, _;
          if (_ = v && 0 < v.remainingCapacity) {
            if (_ = "<" + ("" + l).replace(
              Fa,
              Gt
            ) + ">; rel=preconnect", typeof a == "string") {
              var m = ("" + a).replace(
                Dt,
                Ki
              );
              _ += '; crossorigin="' + m + '"';
            }
            _ = (C = _, 0 <= (v.remainingCapacity -= C.length + 2));
          }
          _ ? (x.resets.connect[E][l] = null, v.preconnects && (v.preconnects += ", "), v.preconnects += C) : (E = [], In(E, {
            rel: "preconnect",
            href: l,
            crossOrigin: a
          }), x.preconnects.add(E));
        }
        ia(s);
      }
    } else El.C(l, a);
  }
  function Kl(l, a, s) {
    var v = ze || null;
    if (v) {
      var x = v.resumableState, E = v.renderState;
      if (a && l) {
        switch (a) {
          case "image":
            if (s)
              var C = s.imageSrcSet, _ = s.imageSizes, m = s.fetchPriority;
            var D = C ? C + `
` + (_ || "") : l;
            if (x.imageResources.hasOwnProperty(D)) return;
            x.imageResources[D] = yt, x = E.headers;
            var U;
            x && 0 < x.remainingCapacity && typeof C != "string" && m === "high" && (U = Nc(l, a, s), 0 <= (x.remainingCapacity -= U.length + 2)) ? (E.resets.image[D] = yt, x.highImagePreloads && (x.highImagePreloads += ", "), x.highImagePreloads += U) : (x = [], In(
              x,
              Ze(
                { rel: "preload", href: C ? void 0 : l, as: a },
                s
              )
            ), m === "high" ? E.highImagePreloads.add(x) : (E.bulkPreloads.add(x), E.preloads.images.set(D, x)));
            break;
          case "style":
            if (x.styleResources.hasOwnProperty(l)) return;
            C = [], In(
              C,
              Ze({ rel: "preload", href: l, as: a }, s)
            ), x.styleResources[l] = !s || typeof s.crossOrigin != "string" && typeof s.integrity != "string" ? yt : [s.crossOrigin, s.integrity], E.preloads.stylesheets.set(l, C), E.bulkPreloads.add(C);
            break;
          case "script":
            if (x.scriptResources.hasOwnProperty(l)) return;
            C = [], E.preloads.scripts.set(l, C), E.bulkPreloads.add(C), In(
              C,
              Ze({ rel: "preload", href: l, as: a }, s)
            ), x.scriptResources[l] = !s || typeof s.crossOrigin != "string" && typeof s.integrity != "string" ? yt : [s.crossOrigin, s.integrity];
            break;
          default:
            if (x.unknownResources.hasOwnProperty(a)) {
              if (C = x.unknownResources[a], C.hasOwnProperty(l))
                return;
            } else
              C = {}, x.unknownResources[a] = C;
            if (C[l] = yt, (x = E.headers) && 0 < x.remainingCapacity && a === "font" && (D = Nc(l, a, s), 0 <= (x.remainingCapacity -= D.length + 2)))
              E.resets.font[l] = yt, x.fontPreloads && (x.fontPreloads += ", "), x.fontPreloads += D;
            else
              switch (x = [], l = Ze({ rel: "preload", href: l, as: a }, s), In(x, l), a) {
                case "font":
                  E.fontPreloads.add(x);
                  break;
                default:
                  E.bulkPreloads.add(x);
              }
        }
        ia(v);
      }
    } else El.L(l, a, s);
  }
  function rc(l, a) {
    var s = ze || null;
    if (s) {
      var v = s.resumableState, x = s.renderState;
      if (l) {
        var E = a && typeof a.as == "string" ? a.as : "script";
        switch (E) {
          case "script":
            if (v.moduleScriptResources.hasOwnProperty(l)) return;
            E = [], v.moduleScriptResources[l] = !a || typeof a.crossOrigin != "string" && typeof a.integrity != "string" ? yt : [a.crossOrigin, a.integrity], x.preloads.moduleScripts.set(l, E);
            break;
          default:
            if (v.moduleUnknownResources.hasOwnProperty(E)) {
              var C = v.unknownResources[E];
              if (C.hasOwnProperty(l)) return;
            } else
              C = {}, v.moduleUnknownResources[E] = C;
            E = [], C[l] = yt;
        }
        In(E, Ze({ rel: "modulepreload", href: l }, a)), x.bulkPreloads.add(E), ia(s);
      }
    } else El.m(l, a);
  }
  function Yt(l, a, s) {
    var v = ze || null;
    if (v) {
      var x = v.resumableState, E = v.renderState;
      if (l) {
        a = a || "default";
        var C = E.styles.get(a), _ = x.styleResources.hasOwnProperty(l) ? x.styleResources[l] : void 0;
        _ !== null && (x.styleResources[l] = null, C || (C = {
          precedence: le(Fe(a)),
          rules: [],
          hrefs: [],
          sheets: /* @__PURE__ */ new Map()
        }, E.styles.set(a, C)), a = {
          state: 0,
          props: Ze(
            { rel: "stylesheet", href: l, "data-precedence": a },
            s
          )
        }, _ && (_.length === 2 && Pa(a.props, _), (E = E.preloads.stylesheets.get(l)) && 0 < E.length ? E.length = 0 : a.state = 1), C.sheets.set(l, a), ia(v));
      }
    } else El.S(l, a, s);
  }
  function wi(l, a) {
    var s = ze || null;
    if (s) {
      var v = s.resumableState, x = s.renderState;
      if (l) {
        var E = v.scriptResources.hasOwnProperty(l) ? v.scriptResources[l] : void 0;
        E !== null && (v.scriptResources[l] = null, a = Ze({ src: l, async: !0 }, a), E && (E.length === 2 && Pa(a, E), l = x.preloads.scripts.get(l)) && (l.length = 0), l = [], x.scripts.add(l), lo(l, a), ia(s));
      }
    } else El.X(l, a);
  }
  function es(l, a) {
    var s = ze || null;
    if (s) {
      var v = s.resumableState, x = s.renderState;
      if (l) {
        var E = v.moduleScriptResources.hasOwnProperty(
          l
        ) ? v.moduleScriptResources[l] : void 0;
        E !== null && (v.moduleScriptResources[l] = null, a = Ze({ src: l, type: "module", async: !0 }, a), E && (E.length === 2 && Pa(a, E), l = x.preloads.moduleScripts.get(l)) && (l.length = 0), l = [], x.scripts.add(l), lo(l, a), ia(s));
      }
    } else El.M(l, a);
  }
  function Pa(l, a) {
    l.crossOrigin == null && (l.crossOrigin = a[0]), l.integrity == null && (l.integrity = a[1]);
  }
  function Nc(l, a, s) {
    l = ("" + l).replace(
      Fa,
      Gt
    ), a = ("" + a).replace(
      Dt,
      Ki
    ), a = "<" + l + '>; rel=preload; as="' + a + '"';
    for (var v in s)
      He.call(s, v) && (l = s[v], typeof l == "string" && (a += "; " + v.toLowerCase() + '="' + ("" + l).replace(
        Dt,
        Ki
      ) + '"'));
    return a;
  }
  var Fa = /[<>\r\n]/g;
  function Gt(l) {
    switch (l) {
      case "<":
        return "%3C";
      case ">":
        return "%3E";
      case `
`:
        return "%0A";
      case "\r":
        return "%0D";
      default:
        throw Error(
          "escapeLinkHrefForHeaderContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
        );
    }
  }
  var Dt = /["';,\r\n]/g;
  function Ki(l) {
    switch (l) {
      case '"':
        return "%22";
      case "'":
        return "%27";
      case ";":
        return "%3B";
      case ",":
        return "%2C";
      case `
`:
        return "%0A";
      case "\r":
        return "%0D";
      default:
        throw Error(
          "escapeStringForLinkHeaderQuotedParamValueContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
        );
    }
  }
  function Pn(l) {
    this.styles.add(l);
  }
  function gn(l) {
    this.stylesheets.add(l);
  }
  function ma(l, a) {
    a.styles.forEach(Pn, l), a.stylesheets.forEach(gn, l), a.suspenseyImages && (l.suspenseyImages = !0);
  }
  function go(l) {
    return 0 < l.stylesheets.size || l.suspenseyImages;
  }
  var Wc = Function.prototype.bind, vo = Symbol.for("react.client.reference");
  function Pu(l) {
    if (l == null) return null;
    if (typeof l == "function")
      return l.$$typeof === vo ? null : l.displayName || l.name || null;
    if (typeof l == "string") return l;
    switch (l) {
      case kn:
        return "Fragment";
      case Kn:
        return "Profiler";
      case Qe:
        return "StrictMode";
      case H:
        return "Suspense";
      case pe:
        return "SuspenseList";
      case Zr:
        return "Activity";
    }
    if (typeof l == "object")
      switch (l.$$typeof) {
        case Qn:
          return "Portal";
        case Et:
          return l.displayName || "Context";
        case Nl:
          return (l._context.displayName || "Context") + ".Consumer";
        case jt:
          var a = l.render;
          return l = l.displayName, l || (l = a.displayName || a.name || "", l = l !== "" ? "ForwardRef(" + l + ")" : "ForwardRef"), l;
        case Ar:
          return a = l.displayName || null, a !== null ? a : Pu(l.type) || "Memo";
        case ne:
          a = l._payload, l = l._init;
          try {
            return Pu(l(a));
          } catch {
          }
      }
    return null;
  }
  var ys = {}, Wn = null;
  function pi(l, a) {
    if (l !== a) {
      l.context._currentValue = l.parentValue, l = l.parent;
      var s = a.parent;
      if (l === null) {
        if (s !== null) throw Error(G(401));
      } else {
        if (s === null) throw Error(G(401));
        pi(l, s);
      }
      a.context._currentValue = a.value;
    }
  }
  function ns(l) {
    l.context._currentValue = l.parentValue, l = l.parent, l !== null && ns(l);
  }
  function i(l) {
    var a = l.parent;
    a !== null && i(a), l.context._currentValue = l.value;
  }
  function o(l, a) {
    if (l.context._currentValue = l.parentValue, l = l.parent, l === null) throw Error(G(402));
    l.depth === a.depth ? pi(l, a) : o(l, a);
  }
  function f(l, a) {
    var s = a.parent;
    if (s === null) throw Error(G(402));
    l.depth === s.depth ? pi(l, s) : f(l, s), a.context._currentValue = a.value;
  }
  function g(l) {
    var a = Wn;
    a !== l && (a === null ? i(l) : l === null ? ns(a) : a.depth === l.depth ? pi(a, l) : a.depth > l.depth ? o(a, l) : f(a, l), Wn = l);
  }
  var T = {
    enqueueSetState: function(l, a) {
      l = l._reactInternals, l.queue !== null && l.queue.push(a);
    },
    enqueueReplaceState: function(l, a) {
      l = l._reactInternals, l.replace = !0, l.queue = [a];
    },
    enqueueForceUpdate: function() {
    }
  }, R = { id: 1, overflow: "" };
  function S(l, a, s) {
    var v = l.id;
    l = l.overflow;
    var x = 32 - Y(v) - 1;
    v &= ~(1 << x), s += 1;
    var E = 32 - Y(a) + x;
    if (30 < E) {
      var C = x - x % 5;
      return E = (v & (1 << C) - 1).toString(32), v >>= C, x -= C, {
        id: 1 << 32 - Y(a) + x | s << x | v,
        overflow: E + l
      };
    }
    return {
      id: 1 << E | s << x | v,
      overflow: l
    };
  }
  var Y = Math.clz32 ? Math.clz32 : q, I = Math.log, W = Math.LN2;
  function q(l) {
    return l >>>= 0, l === 0 ? 32 : 31 - (I(l) / W | 0) | 0;
  }
  function Q() {
  }
  var ae = Error(G(460));
  function ve(l, a, s) {
    switch (s = l[s], s === void 0 ? l.push(a) : s !== a && (a.then(Q, Q), a = s), a.status) {
      case "fulfilled":
        return a.value;
      case "rejected":
        throw a.reason;
      default:
        switch (typeof a.status == "string" ? a.then(Q, Q) : (l = a, l.status = "pending", l.then(
          function(v) {
            if (a.status === "pending") {
              var x = a;
              x.status = "fulfilled", x.value = v;
            }
          },
          function(v) {
            if (a.status === "pending") {
              var x = a;
              x.status = "rejected", x.reason = v;
            }
          }
        )), a.status) {
          case "fulfilled":
            return a.value;
          case "rejected":
            throw a.reason;
        }
        throw De = a, ae;
    }
  }
  var De = null;
  function ke() {
    if (De === null) throw Error(G(459));
    var l = De;
    return De = null, l;
  }
  function xe(l, a) {
    return l === a && (l !== 0 || 1 / l === 1 / a) || l !== l && a !== a;
  }
  var me = typeof Object.is == "function" ? Object.is : xe, Ce = null, _n = null, Le = null, Je = null, on = null, Me = null, Hn = !1, nn = !1, nr = 0, sn = 0, Ke = -1, tn = 0, Ae = null, rn = null, Ft = 0;
  function Be() {
    if (Ce === null)
      throw Error(G(321));
    return Ce;
  }
  function qi() {
    if (0 < Ft) throw Error(G(312));
    return { memoizedState: null, queue: null, next: null };
  }
  function Ei() {
    return Me === null ? on === null ? (Hn = !1, on = Me = qi()) : (Hn = !0, Me = on) : Me.next === null ? (Hn = !1, Me = Me.next = qi()) : (Hn = !0, Me = Me.next), Me;
  }
  function tr() {
    var l = Ae;
    return Ae = null, l;
  }
  function il() {
    Je = Le = _n = Ce = null, nn = !1, on = null, Ft = 0, Me = rn = null;
  }
  function al(l, a) {
    return typeof a == "function" ? a(l) : a;
  }
  function ol(l, a, s) {
    if (Ce = Be(), Me = Ei(), Hn) {
      var v = Me.queue;
      if (a = v.dispatch, rn !== null && (s = rn.get(v), s !== void 0)) {
        rn.delete(v), v = Me.memoizedState;
        do
          v = l(v, s.action), s = s.next;
        while (s !== null);
        return Me.memoizedState = v, [v, a];
      }
      return [Me.memoizedState, a];
    }
    return l = l === al ? typeof a == "function" ? a() : a : s !== void 0 ? s(a) : a, Me.memoizedState = l, l = Me.queue = { last: null, dispatch: null }, l = l.dispatch = Ri.bind(
      null,
      Ce,
      l
    ), [Me.memoizedState, l];
  }
  function ql(l, a) {
    if (Ce = Be(), Me = Ei(), a = a === void 0 ? null : a, Me !== null) {
      var s = Me.memoizedState;
      if (s !== null && a !== null) {
        var v = s[1];
        e: if (v === null) v = !1;
        else {
          for (var x = 0; x < v.length && x < a.length; x++)
            if (!me(a[x], v[x])) {
              v = !1;
              break e;
            }
          v = !0;
        }
        if (v) return s[0];
      }
    }
    return l = l(), Me.memoizedState = [l, a], l;
  }
  function Ri(l, a, s) {
    if (25 <= Ft) throw Error(G(301));
    if (l === Ce)
      if (nn = !0, l = { action: s, next: null }, rn === null && (rn = /* @__PURE__ */ new Map()), s = rn.get(a), s === void 0)
        rn.set(a, l);
      else {
        for (a = s; a.next !== null; ) a = a.next;
        a.next = l;
      }
  }
  function wt() {
    throw Error(G(440));
  }
  function Aa() {
    throw Error(G(394));
  }
  function Ia() {
    throw Error(G(479));
  }
  function lc(l, a, s) {
    Be();
    var v = sn++, x = Le;
    if (typeof l.$$FORM_ACTION == "function") {
      var E = null, C = Je;
      x = x.formState;
      var _ = l.$$IS_SIGNATURE_EQUAL;
      if (x !== null && typeof _ == "function") {
        var m = x[1];
        _.call(l, x[2], x[3]) && (E = s !== void 0 ? "p" + s : "k" + Ir(
          JSON.stringify([C, null, v]),
          0
        ), m === E && (Ke = v, a = x[0]));
      }
      var D = l.bind(null, a);
      return l = function(Z) {
        D(Z);
      }, typeof D.$$FORM_ACTION == "function" && (l.$$FORM_ACTION = function(Z) {
        Z = D.$$FORM_ACTION(Z), s !== void 0 && (s += "", Z.action = s);
        var se = Z.data;
        return se && (E === null && (E = s !== void 0 ? "p" + s : "k" + Ir(
          JSON.stringify([
            C,
            null,
            v
          ]),
          0
        )), se.append("$ACTION_KEY", E)), Z;
      }), [a, l, !1];
    }
    var U = l.bind(null, a);
    return [
      a,
      function(Z) {
        U(Z);
      },
      !1
    ];
  }
  function Hc(l) {
    var a = tn;
    return tn += 1, Ae === null && (Ae = []), ve(Ae, l, a);
  }
  function bo() {
    throw Error(G(393));
  }
  var ji = {
    readContext: function(l) {
      return l._currentValue;
    },
    use: function(l) {
      if (l !== null && typeof l == "object") {
        if (typeof l.then == "function") return Hc(l);
        if (l.$$typeof === Et) return l._currentValue;
      }
      throw Error(G(438, String(l)));
    },
    useContext: function(l) {
      return Be(), l._currentValue;
    },
    useMemo: ql,
    useReducer: ol,
    useRef: function(l) {
      Ce = Be(), Me = Ei();
      var a = Me.memoizedState;
      return a === null ? (l = { current: l }, Me.memoizedState = l) : a;
    },
    useState: function(l) {
      return ol(al, l);
    },
    useInsertionEffect: Q,
    useLayoutEffect: Q,
    useCallback: function(l, a) {
      return ql(function() {
        return l;
      }, a);
    },
    useImperativeHandle: Q,
    useEffect: Q,
    useDebugValue: Q,
    useDeferredValue: function(l, a) {
      return Be(), a !== void 0 ? a : l;
    },
    useTransition: function() {
      return Be(), [!1, Aa];
    },
    useId: function() {
      var l = _n.treeContext, a = l.overflow;
      l = l.id, l = (l & ~(1 << 32 - Y(l) - 1)).toString(32) + a;
      var s = rr;
      if (s === null) throw Error(G(404));
      return a = nr++, l = "_" + s.idPrefix + "R_" + l, 0 < a && (l += "H" + a.toString(32)), l + "_";
    },
    useSyncExternalStore: function(l, a, s) {
      if (s === void 0)
        throw Error(G(407));
      return s();
    },
    useOptimistic: function(l) {
      return Be(), [l, Ia];
    },
    useActionState: lc,
    useFormState: lc,
    useHostTransitionStatus: function() {
      return Be(), xr;
    },
    useMemoCache: function(l) {
      for (var a = Array(l), s = 0; s < l; s++)
        a[s] = _i;
      return a;
    },
    useCacheRefresh: function() {
      return bo;
    },
    useEffectEvent: function() {
      return wt;
    }
  }, rr = null, ic = {
    getCacheForType: function() {
      throw Error(G(248));
    },
    cacheSignal: function() {
      throw Error(G(248));
    }
  }, lr, $r;
  function Ci(l) {
    if (lr === void 0)
      try {
        throw Error();
      } catch (s) {
        var a = s.stack.trim().match(/\n( *(at )?)/);
        lr = a && a[1] || "", $r = -1 < s.stack.indexOf(`
    at`) ? " (<anonymous>)" : -1 < s.stack.indexOf("@") ? "@unknown:0:0" : "";
      }
    return `
` + lr + l + $r;
  }
  var ac = !1;
  function ki(l, a) {
    if (!l || ac) return "";
    ac = !0;
    var s = Error.prepareStackTrace;
    Error.prepareStackTrace = void 0;
    try {
      var v = {
        DetermineComponentFrameRoot: function() {
          try {
            if (a) {
              var Z = function() {
                throw Error();
              };
              if (Object.defineProperty(Z.prototype, "props", {
                set: function() {
                  throw Error();
                }
              }), typeof Reflect == "object" && Reflect.construct) {
                try {
                  Reflect.construct(Z, []);
                } catch (ce) {
                  var se = ce;
                }
                Reflect.construct(l, [], Z);
              } else {
                try {
                  Z.call();
                } catch (ce) {
                  se = ce;
                }
                l.call(Z.prototype);
              }
            } else {
              try {
                throw Error();
              } catch (ce) {
                se = ce;
              }
              (Z = l()) && typeof Z.catch == "function" && Z.catch(function() {
              });
            }
          } catch (ce) {
            if (ce && se && typeof ce.stack == "string")
              return [ce.stack, se.stack];
          }
          return [null, null];
        }
      };
      v.DetermineComponentFrameRoot.displayName = "DetermineComponentFrameRoot";
      var x = Object.getOwnPropertyDescriptor(
        v.DetermineComponentFrameRoot,
        "name"
      );
      x && x.configurable && Object.defineProperty(
        v.DetermineComponentFrameRoot,
        "name",
        { value: "DetermineComponentFrameRoot" }
      );
      var E = v.DetermineComponentFrameRoot(), C = E[0], _ = E[1];
      if (C && _) {
        var m = C.split(`
`), D = _.split(`
`);
        for (x = v = 0; v < m.length && !m[v].includes("DetermineComponentFrameRoot"); )
          v++;
        for (; x < D.length && !D[x].includes(
          "DetermineComponentFrameRoot"
        ); )
          x++;
        if (v === m.length || x === D.length)
          for (v = m.length - 1, x = D.length - 1; 1 <= v && 0 <= x && m[v] !== D[x]; )
            x--;
        for (; 1 <= v && 0 <= x; v--, x--)
          if (m[v] !== D[x]) {
            if (v !== 1 || x !== 1)
              do
                if (v--, x--, 0 > x || m[v] !== D[x]) {
                  var U = `
` + m[v].replace(" at new ", " at ");
                  return l.displayName && U.includes("<anonymous>") && (U = U.replace("<anonymous>", l.displayName)), U;
                }
              while (1 <= v && 0 <= x);
            break;
          }
      }
    } finally {
      ac = !1, Error.prepareStackTrace = s;
    }
    return (s = l ? l.displayName || l.name : "") ? Ci(s) : "";
  }
  function Uc(l) {
    if (typeof l == "string") return Ci(l);
    if (typeof l == "function")
      return l.prototype && l.prototype.isReactComponent ? ki(l, !0) : ki(l, !1);
    if (typeof l == "object" && l !== null) {
      switch (l.$$typeof) {
        case jt:
          return ki(l.render, !1);
        case Ar:
          return ki(l.type, !1);
        case ne:
          var a = l, s = a._payload;
          a = a._init;
          try {
            l = a(s);
          } catch {
            return Ci("Lazy");
          }
          return Uc(l);
      }
      if (typeof l.name == "string") {
        e: {
          s = l.name, a = l.env;
          var v = l.debugLocation;
          if (v != null && (l = Error.prepareStackTrace, Error.prepareStackTrace = void 0, v = v.stack, Error.prepareStackTrace = l, v.startsWith(`Error: react-stack-top-frame
`) && (v = v.slice(29)), l = v.indexOf(`
`), l !== -1 && (v = v.slice(l + 1)), l = v.indexOf("react_stack_bottom_frame"), l !== -1 && (l = v.lastIndexOf(`
`, l)), l = l !== -1 ? v = v.slice(0, l) : "", v = l.lastIndexOf(`
`), l = v === -1 ? l : l.slice(v + 1), l.indexOf(s) !== -1)) {
            s = `
` + l;
            break e;
          }
          s = Ci(
            s + (a ? " [" + a + "]" : "")
          );
        }
        return s;
      }
    }
    switch (l) {
      case pe:
        return Ci("SuspenseList");
      case H:
        return Ci("Suspense");
    }
    return "";
  }
  function ct(l, a) {
    return (500 < a.byteSize || go(a.contentState)) && a.contentPreamble === null;
  }
  function oc(l) {
    if (typeof l == "object" && l !== null && typeof l.environmentName == "string") {
      var a = l.environmentName;
      l = [l].slice(0), typeof l[0] == "string" ? l.splice(
        0,
        1,
        "%c%s%c " + l[0],
        "background: #e6e6e6;background: light-dark(rgba(0,0,0,0.1), rgba(255,255,255,0.25));color: #000000;color: light-dark(#000000, #ffffff);border-radius: 2px",
        " " + a + " ",
        ""
      ) : l.splice(
        0,
        0,
        "%c%s%c",
        "background: #e6e6e6;background: light-dark(rgba(0,0,0,0.1), rgba(255,255,255,0.25));color: #000000;color: light-dark(#000000, #ffffff);border-radius: 2px",
        " " + a + " ",
        ""
      ), l.unshift(console), a = Wc.apply(console.error, l), a();
    } else console.error(l);
    return null;
  }
  function $i(l, a, s, v, x, E, C, _, m, D, U) {
    var Z = /* @__PURE__ */ new Set();
    this.destination = null, this.flushScheduled = !1, this.resumableState = l, this.renderState = a, this.rootFormatContext = s, this.progressiveChunkSize = v === void 0 ? 12800 : v, this.status = 10, this.fatalError = null, this.pendingRootTasks = this.allPendingTasks = this.nextSegmentId = 0, this.completedPreambleSegments = this.completedRootSegment = null, this.byteSize = 0, this.abortableTasks = Z, this.pingedTasks = [], this.clientRenderedBoundaries = [], this.completedBoundaries = [], this.partialBoundaries = [], this.trackedPostpones = null, this.onError = x === void 0 ? oc : x, this.onPostpone = D === void 0 ? Q : D, this.onAllReady = E === void 0 ? Q : E, this.onShellReady = C === void 0 ? Q : C, this.onShellError = _ === void 0 ? Q : _, this.onFatalError = m === void 0 ? Q : m, this.formState = U === void 0 ? null : U;
  }
  function yo(l, a, s, v, x, E, C, _, m, D, U, Z) {
    return a = new $i(
      a,
      s,
      v,
      x,
      E,
      C,
      _,
      m,
      D,
      U,
      Z
    ), s = Cr(
      a,
      0,
      null,
      v,
      !1,
      !1
    ), s.parentFlushed = !0, l = ea(
      a,
      null,
      l,
      -1,
      null,
      s,
      null,
      null,
      a.abortableTasks,
      null,
      v,
      null,
      R,
      null,
      null
    ), na(l), a.pingedTasks.push(l), a;
  }
  function Ma(l, a, s, v, x, E, C, _, m, D, U) {
    return l = yo(
      l,
      a,
      s,
      v,
      x,
      E,
      C,
      _,
      m,
      D,
      U,
      void 0
    ), l.trackedPostpones = {
      workingMap: /* @__PURE__ */ new Map(),
      rootNodes: [],
      rootSlots: null
    }, l;
  }
  function nt(l, a, s, v, x, E, C, _, m) {
    return s = new $i(
      a.resumableState,
      s,
      a.rootFormatContext,
      a.progressiveChunkSize,
      v,
      x,
      E,
      C,
      _,
      m,
      null
    ), s.nextSegmentId = a.nextSegmentId, typeof a.replaySlots == "number" ? (v = Cr(
      s,
      0,
      null,
      a.rootFormatContext,
      !1,
      !1
    ), v.parentFlushed = !0, l = ea(
      s,
      null,
      l,
      -1,
      null,
      v,
      null,
      null,
      s.abortableTasks,
      null,
      a.rootFormatContext,
      null,
      R,
      null,
      null
    ), na(l), s.pingedTasks.push(l), s) : (l = Un(
      s,
      null,
      {
        nodes: a.replayNodes,
        slots: a.replaySlots,
        pendingTasks: 0
      },
      l,
      -1,
      null,
      null,
      s.abortableTasks,
      null,
      a.rootFormatContext,
      null,
      R,
      null,
      null
    ), na(l), s.pingedTasks.push(l), s);
  }
  function el(l, a, s, v, x, E, C, _, m) {
    return l = nt(
      l,
      a,
      s,
      v,
      x,
      E,
      C,
      _,
      m
    ), l.trackedPostpones = {
      workingMap: /* @__PURE__ */ new Map(),
      rootNodes: [],
      rootSlots: null
    }, l;
  }
  var ze = null;
  function Si(l, a) {
    l.pingedTasks.push(a), l.pingedTasks.length === 1 && (l.flushScheduled = l.destination !== null, l.trackedPostpones !== null || l.status === 10 ? Li(function() {
      return ts(l);
    }) : bt(function() {
      return ts(l);
    }));
  }
  function xo(l, a, s, v, x) {
    return s = {
      status: 0,
      rootSegmentID: -1,
      parentFlushed: !1,
      pendingTasks: 0,
      row: a,
      completedSegments: [],
      byteSize: 0,
      fallbackAbortableTasks: s,
      errorDigest: null,
      contentState: Il(),
      fallbackState: Il(),
      contentPreamble: v,
      fallbackPreamble: x,
      trackedContentKeyPath: null,
      trackedFallbackNode: null
    }, a !== null && (a.pendingTasks++, v = a.boundaries, v !== null && (l.allPendingTasks++, s.pendingTasks++, v.push(s)), l = a.inheritedHoistables, l !== null && ma(s.contentState, l)), s;
  }
  function ea(l, a, s, v, x, E, C, _, m, D, U, Z, se, ce, Ge) {
    l.allPendingTasks++, x === null ? l.pendingRootTasks++ : x.pendingTasks++, ce !== null && ce.pendingTasks++;
    var Se = {
      replay: null,
      node: s,
      childIndex: v,
      ping: function() {
        return Si(l, Se);
      },
      blockedBoundary: x,
      blockedSegment: E,
      blockedPreamble: C,
      hoistableState: _,
      abortSet: m,
      keyPath: D,
      formatContext: U,
      context: Z,
      treeContext: se,
      row: ce,
      componentStack: Ge,
      thenableState: a
    };
    return m.add(Se), Se;
  }
  function Un(l, a, s, v, x, E, C, _, m, D, U, Z, se, ce) {
    l.allPendingTasks++, E === null ? l.pendingRootTasks++ : E.pendingTasks++, se !== null && se.pendingTasks++, s.pendingTasks++;
    var Ge = {
      replay: s,
      node: v,
      childIndex: x,
      ping: function() {
        return Si(l, Ge);
      },
      blockedBoundary: E,
      blockedSegment: null,
      blockedPreamble: null,
      hoistableState: C,
      abortSet: _,
      keyPath: m,
      formatContext: D,
      context: U,
      treeContext: Z,
      row: se,
      componentStack: ce,
      thenableState: a
    };
    return _.add(Ge), Ge;
  }
  function Cr(l, a, s, v, x, E) {
    return {
      status: 0,
      parentFlushed: !1,
      id: -1,
      index: a,
      chunks: [],
      children: [],
      preambleChildren: [],
      parentFormatContext: v,
      boundary: s,
      lastPushedText: x,
      textEmbedded: E
    };
  }
  function na(l) {
    var a = l.node;
    if (typeof a == "object" && a !== null)
      switch (a.$$typeof) {
        case vt:
          l.componentStack = { parent: l.componentStack, type: a.type };
      }
  }
  function Oa(l) {
    return l === null ? null : { parent: l.parent, type: "Suspense Fallback" };
  }
  function cl(l) {
    var a = {};
    return l && Object.defineProperty(a, "componentStack", {
      configurable: !0,
      enumerable: !0,
      get: function() {
        try {
          var s = "", v = l;
          do
            s += Uc(v.type), v = v.parent;
          while (v);
          var x = s;
        } catch (E) {
          x = `
Error generating stack: ` + E.message + `
` + E.stack;
        }
        return Object.defineProperty(a, "componentStack", {
          value: x
        }), x;
      }
    }), a;
  }
  function Xt(l, a, s) {
    if (l = l.onError, a = l(a, s), a == null || typeof a == "string") return a;
  }
  function To(l, a) {
    var s = l.onShellError, v = l.onFatalError;
    s(a), v(a), l.destination !== null ? (l.status = 14, We(l.destination, a)) : (l.status = 13, l.fatalError = a);
  }
  function ir(l, a) {
    Fu(l, a.next, a.hoistables);
  }
  function Fu(l, a, s) {
    for (; a !== null; ) {
      s !== null && (ma(a.hoistables, s), a.inheritedHoistables = s);
      var v = a.boundaries;
      if (v !== null) {
        a.boundaries = null;
        for (var x = 0; x < v.length; x++) {
          var E = v[x];
          s !== null && ma(E.contentState, s), ul(l, E, null, null);
        }
      }
      if (a.pendingTasks--, 0 < a.pendingTasks) break;
      s = a.hoistables, a = a.next;
    }
  }
  function _a(l, a) {
    var s = a.boundaries;
    if (s !== null && a.pendingTasks === s.length) {
      for (var v = !0, x = 0; x < s.length; x++) {
        var E = s[x];
        if (E.pendingTasks !== 1 || E.parentFlushed || ct(l, E)) {
          v = !1;
          break;
        }
      }
      v && Fu(l, a, a.hoistables);
    }
  }
  function cc(l) {
    var a = {
      pendingTasks: 1,
      boundaries: null,
      hoistables: Il(),
      inheritedHoistables: null,
      together: !1,
      next: null
    };
    return l !== null && 0 < l.pendingTasks && (a.pendingTasks++, a.boundaries = [], l.next = a), a;
  }
  function Yc(l, a, s, v, x) {
    var E = a.keyPath, C = a.treeContext, _ = a.row;
    a.keyPath = s, s = v.length;
    var m = null;
    if (a.replay !== null) {
      var D = a.replay.slots;
      if (D !== null && typeof D == "object")
        for (var U = 0; U < s; U++) {
          var Z = x !== "backwards" && x !== "unstable_legacy-backwards" ? U : s - 1 - U, se = v[Z];
          a.row = m = cc(
            m
          ), a.treeContext = S(C, s, Z);
          var ce = D[Z];
          typeof ce == "number" ? (La(l, a, ce, se, Z), delete D[Z]) : ut(l, a, se, Z), --m.pendingTasks === 0 && ir(l, m);
        }
      else
        for (D = 0; D < s; D++)
          U = x !== "backwards" && x !== "unstable_legacy-backwards" ? D : s - 1 - D, Z = v[U], a.row = m = cc(m), a.treeContext = S(C, s, U), ut(l, a, Z, U), --m.pendingTasks === 0 && ir(l, m);
    } else if (x !== "backwards" && x !== "unstable_legacy-backwards")
      for (x = 0; x < s; x++)
        D = v[x], a.row = m = cc(m), a.treeContext = S(
          C,
          s,
          x
        ), ut(l, a, D, x), --m.pendingTasks === 0 && ir(l, m);
    else {
      for (x = a.blockedSegment, D = x.children.length, U = x.chunks.length, Z = s - 1; 0 <= Z; Z--) {
        se = v[Z], a.row = m = cc(
          m
        ), a.treeContext = S(C, s, Z), ce = Cr(
          l,
          U,
          null,
          a.formatContext,
          Z === 0 ? x.lastPushedText : !0,
          !0
        ), x.children.splice(D, 0, ce), a.blockedSegment = ce;
        try {
          ut(l, a, se, Z), ce.lastPushedText && ce.textEmbedded && ce.chunks.push(Hl), ce.status = 1, ta(l, a.blockedBoundary, ce), --m.pendingTasks === 0 && ir(l, m);
        } catch (Ge) {
          throw ce.status = l.status === 12 ? 3 : 4, Ge;
        }
      }
      a.blockedSegment = x, x.lastPushedText = !1;
    }
    _ !== null && m !== null && 0 < m.pendingTasks && (_.pendingTasks++, m.next = _), a.treeContext = C, a.row = _, a.keyPath = E;
  }
  function Gc(l, a, s, v, x, E) {
    var C = a.thenableState;
    for (a.thenableState = null, Ce = {}, _n = a, Le = l, Je = s, sn = nr = 0, Ke = -1, tn = 0, Ae = C, l = v(x, E); nn; )
      nn = !1, sn = nr = 0, Ke = -1, tn = 0, Ft += 1, Me = null, l = v(x, E);
    return il(), l;
  }
  function jl(l, a, s, v, x, E, C) {
    var _ = !1;
    if (E !== 0 && l.formState !== null) {
      var m = a.blockedSegment;
      if (m !== null) {
        _ = !0, m = m.chunks;
        for (var D = 0; D < E; D++)
          D === C ? m.push(yu) : m.push(Ac);
      }
    }
    E = a.keyPath, a.keyPath = s, x ? (s = a.treeContext, a.treeContext = S(s, 1, 0), ut(l, a, v, -1), a.treeContext = s) : _ ? ut(l, a, v, -1) : ar(l, a, v, -1), a.keyPath = E;
  }
  function Da(l, a, s, v, x, E) {
    if (typeof v == "function")
      if (v.prototype && v.prototype.isReactComponent) {
        var C = x;
        if ("ref" in x) {
          C = {};
          for (var _ in x)
            _ !== "ref" && (C[_] = x[_]);
        }
        var m = v.defaultProps;
        if (m) {
          C === x && (C = Ze({}, C, x));
          for (var D in m)
            C[D] === void 0 && (C[D] = m[D]);
        }
        x = C, C = ys, m = v.contextType, typeof m == "object" && m !== null && (C = m._currentValue), C = new v(x, C);
        var U = C.state !== void 0 ? C.state : null;
        if (C.updater = T, C.props = x, C.state = U, m = { queue: [], replace: !1 }, C._reactInternals = m, E = v.contextType, C.context = typeof E == "object" && E !== null ? E._currentValue : ys, E = v.getDerivedStateFromProps, typeof E == "function" && (E = E(x, U), U = E == null ? U : Ze({}, U, E), C.state = U), typeof v.getDerivedStateFromProps != "function" && typeof C.getSnapshotBeforeUpdate != "function" && (typeof C.UNSAFE_componentWillMount == "function" || typeof C.componentWillMount == "function"))
          if (v = C.state, typeof C.componentWillMount == "function" && C.componentWillMount(), typeof C.UNSAFE_componentWillMount == "function" && C.UNSAFE_componentWillMount(), v !== C.state && T.enqueueReplaceState(
            C,
            C.state,
            null
          ), m.queue !== null && 0 < m.queue.length)
            if (v = m.queue, E = m.replace, m.queue = null, m.replace = !1, E && v.length === 1)
              C.state = v[0];
            else {
              for (m = E ? v[0] : C.state, U = !0, E = E ? 1 : 0; E < v.length; E++)
                D = v[E], D = typeof D == "function" ? D.call(C, m, x, void 0) : D, D != null && (U ? (U = !1, m = Ze({}, m, D)) : Ze(m, D));
              C.state = m;
            }
          else m.queue = null;
        if (v = C.render(), l.status === 12) throw null;
        x = a.keyPath, a.keyPath = s, ar(l, a, v, -1), a.keyPath = x;
      } else {
        if (v = Gc(l, a, s, v, x, void 0), l.status === 12) throw null;
        jl(
          l,
          a,
          s,
          v,
          nr !== 0,
          sn,
          Ke
        );
      }
    else if (typeof v == "string")
      if (C = a.blockedSegment, C === null)
        C = x.children, m = a.formatContext, U = a.keyPath, a.formatContext = va(m, v, x), a.keyPath = s, ut(l, a, C, -1), a.formatContext = m, a.keyPath = U;
      else {
        if (U = Jo(
          C.chunks,
          v,
          x,
          l.resumableState,
          l.renderState,
          a.blockedPreamble,
          a.hoistableState,
          a.formatContext,
          C.lastPushedText
        ), C.lastPushedText = !1, m = a.formatContext, E = a.keyPath, a.keyPath = s, (a.formatContext = va(m, v, x)).insertionMode === 3) {
          s = Cr(
            l,
            0,
            null,
            a.formatContext,
            !1,
            !1
          ), C.preambleChildren.push(s), a.blockedSegment = s;
          try {
            s.status = 6, ut(l, a, U, -1), s.lastPushedText && s.textEmbedded && s.chunks.push(Hl), s.status = 1, ta(l, a.blockedBoundary, s);
          } finally {
            a.blockedSegment = C;
          }
        } else ut(l, a, U, -1);
        a.formatContext = m, a.keyPath = E;
        e: {
          switch (a = C.chunks, l = l.resumableState, v) {
            case "title":
            case "style":
            case "script":
            case "area":
            case "base":
            case "br":
            case "col":
            case "embed":
            case "hr":
            case "img":
            case "input":
            case "keygen":
            case "link":
            case "meta":
            case "param":
            case "source":
            case "track":
            case "wbr":
              break e;
            case "body":
              if (1 >= m.insertionMode) {
                l.hasBody = !0;
                break e;
              }
              break;
            case "html":
              if (m.insertionMode === 0) {
                l.hasHtml = !0;
                break e;
              }
              break;
            case "head":
              if (1 >= m.insertionMode) break e;
          }
          a.push(Xl(v));
        }
        C.lastPushedText = !1;
      }
    else {
      switch (v) {
        case zo:
        case Qe:
        case Kn:
        case kn:
          v = a.keyPath, a.keyPath = s, ar(l, a, x.children, -1), a.keyPath = v;
          return;
        case Zr:
          v = a.blockedSegment, v === null ? x.mode !== "hidden" && (v = a.keyPath, a.keyPath = s, ut(l, a, x.children, -1), a.keyPath = v) : x.mode !== "hidden" && (v.chunks.push(wr), v.lastPushedText = !1, C = a.keyPath, a.keyPath = s, ut(l, a, x.children, -1), a.keyPath = C, v.chunks.push(Ko), v.lastPushedText = !1);
          return;
        case pe:
          e: {
            if (v = x.children, x = x.revealOrder, x === "forwards" || x === "backwards" || x === "unstable_legacy-backwards") {
              if (Ee(v)) {
                Yc(l, a, s, v, x);
                break e;
              }
              if ((C = Nt(v)) && (C = C.call(v))) {
                if (m = C.next(), !m.done) {
                  do
                    m = C.next();
                  while (!m.done);
                  Yc(l, a, s, v, x);
                }
                break e;
              }
            }
            x === "together" ? (x = a.keyPath, C = a.row, m = a.row = cc(null), m.boundaries = [], m.together = !0, a.keyPath = s, ar(l, a, v, -1), --m.pendingTasks === 0 && ir(l, m), a.keyPath = x, a.row = C, C !== null && 0 < m.pendingTasks && (C.pendingTasks++, m.next = C)) : (x = a.keyPath, a.keyPath = s, ar(l, a, v, -1), a.keyPath = x);
          }
          return;
        case Bo:
        case N:
          throw Error(G(343));
        case H:
          e: if (a.replay !== null) {
            v = a.keyPath, C = a.formatContext, m = a.row, a.keyPath = s, a.formatContext = ll(
              l.resumableState,
              C
            ), a.row = null, s = x.children;
            try {
              ut(l, a, s, -1);
            } finally {
              a.keyPath = v, a.formatContext = C, a.row = m;
            }
          } else {
            v = a.keyPath, E = a.formatContext;
            var Z = a.row;
            D = a.blockedBoundary, _ = a.blockedPreamble;
            var se = a.hoistableState, ce = a.blockedSegment, Ge = x.fallback;
            x = x.children;
            var Se = /* @__PURE__ */ new Set(), Xe = 2 > a.formatContext.insertionMode ? xo(
              l,
              a.row,
              Se,
              Nn(),
              Nn()
            ) : xo(
              l,
              a.row,
              Se,
              null,
              null
            );
            l.trackedPostpones !== null && (Xe.trackedContentKeyPath = s);
            var Yn = Cr(
              l,
              ce.chunks.length,
              Xe,
              a.formatContext,
              !1,
              !1
            );
            ce.children.push(Yn), ce.lastPushedText = !1;
            var Fn = Cr(
              l,
              0,
              null,
              a.formatContext,
              !1,
              !1
            );
            if (Fn.parentFlushed = !0, l.trackedPostpones !== null) {
              C = a.componentStack, m = [s[0], "Suspense Fallback", s[2]], U = [m[1], m[2], [], null], l.trackedPostpones.workingMap.set(m, U), Xe.trackedFallbackNode = U, a.blockedSegment = Yn, a.blockedPreamble = Xe.fallbackPreamble, a.keyPath = m, a.formatContext = bu(
                l.resumableState,
                E
              ), a.componentStack = Oa(C), Yn.status = 6;
              try {
                ut(l, a, Ge, -1), Yn.lastPushedText && Yn.textEmbedded && Yn.chunks.push(Hl), Yn.status = 1, ta(l, D, Yn);
              } catch (Zt) {
                throw Yn.status = l.status === 12 ? 3 : 4, Zt;
              } finally {
                a.blockedSegment = ce, a.blockedPreamble = _, a.keyPath = v, a.formatContext = E;
              }
              a = ea(
                l,
                null,
                x,
                -1,
                Xe,
                Fn,
                Xe.contentPreamble,
                Xe.contentState,
                a.abortSet,
                s,
                ll(
                  l.resumableState,
                  a.formatContext
                ),
                a.context,
                a.treeContext,
                null,
                C
              ), na(a), l.pingedTasks.push(a);
            } else {
              a.blockedBoundary = Xe, a.blockedPreamble = Xe.contentPreamble, a.hoistableState = Xe.contentState, a.blockedSegment = Fn, a.keyPath = s, a.formatContext = ll(
                l.resumableState,
                E
              ), a.row = null, Fn.status = 6;
              try {
                if (ut(l, a, x, -1), Fn.lastPushedText && Fn.textEmbedded && Fn.chunks.push(Hl), Fn.status = 1, ta(l, Xe, Fn), Qc(Xe, Fn), Xe.pendingTasks === 0 && Xe.status === 0) {
                  if (Xe.status = 1, !ct(l, Xe)) {
                    Z !== null && --Z.pendingTasks === 0 && ir(l, Z), l.pendingRootTasks === 0 && a.blockedPreamble && Ml(l);
                    break e;
                  }
                } else
                  Z !== null && Z.together && _a(l, Z);
              } catch (Zt) {
                Xe.status = 4, l.status === 12 ? (Fn.status = 3, C = l.fatalError) : (Fn.status = 4, C = Zt), m = cl(a.componentStack), U = Xt(
                  l,
                  C,
                  m
                ), Xe.errorDigest = U, Xc(l, Xe);
              } finally {
                a.blockedBoundary = D, a.blockedPreamble = _, a.hoistableState = se, a.blockedSegment = ce, a.keyPath = v, a.formatContext = E, a.row = Z;
              }
              a = ea(
                l,
                null,
                Ge,
                -1,
                D,
                Yn,
                Xe.fallbackPreamble,
                Xe.fallbackState,
                Se,
                [s[0], "Suspense Fallback", s[2]],
                bu(
                  l.resumableState,
                  a.formatContext
                ),
                a.context,
                a.treeContext,
                a.row,
                Oa(
                  a.componentStack
                )
              ), na(a), l.pingedTasks.push(a);
            }
          }
          return;
      }
      if (typeof v == "object" && v !== null)
        switch (v.$$typeof) {
          case jt:
            if ("ref" in x)
              for (ce in C = {}, x)
                ce !== "ref" && (C[ce] = x[ce]);
            else C = x;
            v = Gc(
              l,
              a,
              s,
              v.render,
              C,
              E
            ), jl(
              l,
              a,
              s,
              v,
              nr !== 0,
              sn,
              Ke
            );
            return;
          case Ar:
            Da(l, a, s, v.type, x, E);
            return;
          case Et:
            if (m = x.children, C = a.keyPath, x = x.value, U = v._currentValue, v._currentValue = x, E = Wn, Wn = v = {
              parent: E,
              depth: E === null ? 0 : E.depth + 1,
              context: v,
              parentValue: U,
              value: x
            }, a.context = v, a.keyPath = s, ar(l, a, m, -1), l = Wn, l === null) throw Error(G(403));
            l.context._currentValue = l.parentValue, l = Wn = l.parent, a.context = l, a.keyPath = C;
            return;
          case Nl:
            x = x.children, v = x(v._context._currentValue), x = a.keyPath, a.keyPath = s, ar(l, a, v, -1), a.keyPath = x;
            return;
          case ne:
            if (C = v._init, v = C(v._payload), l.status === 12) throw null;
            Da(l, a, s, v, x, E);
            return;
        }
      throw Error(
        G(130, v == null ? v : typeof v, "")
      );
    }
  }
  function La(l, a, s, v, x) {
    var E = a.replay, C = a.blockedBoundary, _ = Cr(
      l,
      0,
      null,
      a.formatContext,
      !1,
      !1
    );
    _.id = s, _.parentFlushed = !0;
    try {
      a.replay = null, a.blockedSegment = _, ut(l, a, v, x), _.status = 1, ta(l, C, _), C === null ? l.completedRootSegment = _ : (Qc(C, _), C.parentFlushed && l.partialBoundaries.push(C));
    } finally {
      a.replay = E, a.blockedSegment = null;
    }
  }
  function ar(l, a, s, v) {
    a.replay !== null && typeof a.replay.slots == "number" ? La(l, a, a.replay.slots, s, v) : (a.node = s, a.childIndex = v, s = a.componentStack, na(a), uc(l, a), a.componentStack = s);
  }
  function uc(l, a) {
    var s = a.node, v = a.childIndex;
    if (s !== null) {
      if (typeof s == "object") {
        switch (s.$$typeof) {
          case vt:
            var x = s.type, E = s.key, C = s.props;
            s = C.ref;
            var _ = s !== void 0 ? s : null, m = Pu(x), D = E ?? (v === -1 ? 0 : v);
            if (E = [a.keyPath, m, D], a.replay !== null)
              e: {
                var U = a.replay;
                for (v = U.nodes, s = 0; s < v.length; s++) {
                  var Z = v[s];
                  if (D === Z[1]) {
                    if (Z.length === 4) {
                      if (m !== null && m !== Z[0])
                        throw Error(
                          G(490, Z[0], m)
                        );
                      var se = Z[2];
                      m = Z[3], D = a.node, a.replay = {
                        nodes: se,
                        slots: m,
                        pendingTasks: 1
                      };
                      try {
                        if (Da(l, a, E, x, C, _), a.replay.pendingTasks === 1 && 0 < a.replay.nodes.length)
                          throw Error(G(488));
                        a.replay.pendingTasks--;
                      } catch (Pe) {
                        if (typeof Pe == "object" && Pe !== null && (Pe === ae || typeof Pe.then == "function"))
                          throw a.node === D ? a.replay = U : v.splice(s, 1), Pe;
                        a.replay.pendingTasks--, C = cl(a.componentStack), E = l, l = a.blockedBoundary, x = Pe, C = Xt(E, x, C), fc(
                          E,
                          l,
                          se,
                          m,
                          x,
                          C
                        );
                      }
                      a.replay = U;
                    } else {
                      if (x !== H)
                        throw Error(
                          G(
                            490,
                            "Suspense",
                            Pu(x) || "Unknown"
                          )
                        );
                      n: {
                        U = void 0, x = Z[5], _ = Z[2], m = Z[3], D = Z[4] === null ? [] : Z[4][2], Z = Z[4] === null ? null : Z[4][3];
                        var ce = a.keyPath, Ge = a.formatContext, Se = a.row, Xe = a.replay, Yn = a.blockedBoundary, Fn = a.hoistableState, Zt = C.children, Dr = C.fallback, fl = /* @__PURE__ */ new Set();
                        C = 2 > a.formatContext.insertionMode ? xo(
                          l,
                          a.row,
                          fl,
                          Nn(),
                          Nn()
                        ) : xo(
                          l,
                          a.row,
                          fl,
                          null,
                          null
                        ), C.parentFlushed = !0, C.rootSegmentID = x, a.blockedBoundary = C, a.hoistableState = C.contentState, a.keyPath = E, a.formatContext = ll(
                          l.resumableState,
                          Ge
                        ), a.row = null, a.replay = {
                          nodes: _,
                          slots: m,
                          pendingTasks: 1
                        };
                        try {
                          if (ut(l, a, Zt, -1), a.replay.pendingTasks === 1 && 0 < a.replay.nodes.length)
                            throw Error(G(488));
                          if (a.replay.pendingTasks--, C.pendingTasks === 0 && C.status === 0) {
                            C.status = 1, l.completedBoundaries.push(C);
                            break n;
                          }
                        } catch (Pe) {
                          C.status = 4, se = cl(a.componentStack), U = Xt(
                            l,
                            Pe,
                            se
                          ), C.errorDigest = U, a.replay.pendingTasks--, l.clientRenderedBoundaries.push(C);
                        } finally {
                          a.blockedBoundary = Yn, a.hoistableState = Fn, a.replay = Xe, a.keyPath = ce, a.formatContext = Ge, a.row = Se;
                        }
                        se = Un(
                          l,
                          null,
                          {
                            nodes: D,
                            slots: Z,
                            pendingTasks: 0
                          },
                          Dr,
                          -1,
                          Yn,
                          C.fallbackState,
                          fl,
                          [E[0], "Suspense Fallback", E[2]],
                          bu(
                            l.resumableState,
                            a.formatContext
                          ),
                          a.context,
                          a.treeContext,
                          a.row,
                          Oa(
                            a.componentStack
                          )
                        ), na(se), l.pingedTasks.push(se);
                      }
                    }
                    v.splice(s, 1);
                    break e;
                  }
                }
              }
            else Da(l, a, E, x, C, _);
            return;
          case Qn:
            throw Error(G(257));
          case ne:
            if (se = s._init, s = se(s._payload), l.status === 12) throw null;
            ar(l, a, s, v);
            return;
        }
        if (Ee(s)) {
          sc(l, a, s, v);
          return;
        }
        if ((se = Nt(s)) && (se = se.call(s))) {
          if (s = se.next(), !s.done) {
            C = [];
            do
              C.push(s.value), s = se.next();
            while (!s.done);
            sc(l, a, C, v);
          }
          return;
        }
        if (typeof s.then == "function")
          return a.thenableState = null, ar(l, a, Hc(s), v);
        if (s.$$typeof === Et)
          return ar(
            l,
            a,
            s._currentValue,
            v
          );
        throw v = Object.prototype.toString.call(s), Error(
          G(
            31,
            v === "[object Object]" ? "object with keys {" + Object.keys(s).join(", ") + "}" : v
          )
        );
      }
      typeof s == "string" ? (v = a.blockedSegment, v !== null && (v.lastPushedText = Ju(
        v.chunks,
        s,
        l.renderState,
        v.lastPushedText
      ))) : (typeof s == "number" || typeof s == "bigint") && (v = a.blockedSegment, v !== null && (v.lastPushedText = Ju(
        v.chunks,
        "" + s,
        l.renderState,
        v.lastPushedText
      )));
    }
  }
  function sc(l, a, s, v) {
    var x = a.keyPath;
    if (v !== -1 && (a.keyPath = [a.keyPath, "Fragment", v], a.replay !== null)) {
      for (var E = a.replay, C = E.nodes, _ = 0; _ < C.length; _++) {
        var m = C[_];
        if (m[1] === v) {
          v = m[2], m = m[3], a.replay = { nodes: v, slots: m, pendingTasks: 1 };
          try {
            if (sc(l, a, s, -1), a.replay.pendingTasks === 1 && 0 < a.replay.nodes.length)
              throw Error(G(488));
            a.replay.pendingTasks--;
          } catch (Z) {
            if (typeof Z == "object" && Z !== null && (Z === ae || typeof Z.then == "function"))
              throw Z;
            a.replay.pendingTasks--, s = cl(a.componentStack);
            var D = a.blockedBoundary, U = Z;
            s = Xt(l, U, s), fc(
              l,
              D,
              v,
              m,
              U,
              s
            );
          }
          a.replay = E, C.splice(_, 1);
          break;
        }
      }
      a.keyPath = x;
      return;
    }
    if (E = a.treeContext, C = s.length, a.replay !== null && (_ = a.replay.slots, _ !== null && typeof _ == "object")) {
      for (v = 0; v < C; v++)
        m = s[v], a.treeContext = S(E, C, v), D = _[v], typeof D == "number" ? (La(l, a, D, m, v), delete _[v]) : ut(l, a, m, v);
      a.treeContext = E, a.keyPath = x;
      return;
    }
    for (_ = 0; _ < C; _++)
      v = s[_], a.treeContext = S(E, C, _), ut(l, a, v, _);
    a.treeContext = E, a.keyPath = x;
  }
  function mu(l, a, s) {
    if (s.status = 5, s.rootSegmentID = l.nextSegmentId++, l = s.trackedContentKeyPath, l === null) throw Error(G(486));
    var v = s.trackedFallbackNode, x = [], E = a.workingMap.get(l);
    return E === void 0 ? (s = [
      l[1],
      l[2],
      x,
      null,
      v,
      s.rootSegmentID
    ], a.workingMap.set(l, s), _u(s, l[0], a), s) : (E[4] = v, E[5] = s.rootSegmentID, E);
  }
  function Au(l, a, s, v) {
    v.status = 5;
    var x = s.keyPath, E = s.blockedBoundary;
    if (E === null)
      v.id = l.nextSegmentId++, a.rootSlots = v.id, l.completedRootSegment !== null && (l.completedRootSegment.status = 5);
    else {
      if (E !== null && E.status === 0) {
        var C = mu(
          l,
          a,
          E
        );
        if (E.trackedContentKeyPath === x && s.childIndex === -1) {
          v.id === -1 && (v.id = v.parentFlushed ? E.rootSegmentID : l.nextSegmentId++), C[3] = v.id;
          return;
        }
      }
      if (v.id === -1 && (v.id = v.parentFlushed && E !== null ? E.rootSegmentID : l.nextSegmentId++), s.childIndex === -1)
        x === null ? a.rootSlots = v.id : (s = a.workingMap.get(x), s === void 0 ? (s = [x[1], x[2], [], v.id], _u(s, x[0], a)) : s[3] = v.id);
      else {
        if (x === null) {
          if (l = a.rootSlots, l === null)
            l = a.rootSlots = {};
          else if (typeof l == "number")
            throw Error(G(491));
        } else if (E = a.workingMap, C = E.get(x), C === void 0)
          l = {}, C = [x[1], x[2], [], l], E.set(x, C), _u(C, x[0], a);
        else if (l = C[3], l === null)
          l = C[3] = {};
        else if (typeof l == "number")
          throw Error(G(491));
        l[s.childIndex] = v.id;
      }
    }
  }
  function Xc(l, a) {
    l = l.trackedPostpones, l !== null && (a = a.trackedContentKeyPath, a !== null && (a = l.workingMap.get(a), a !== void 0 && (a.length = 4, a[2] = [], a[3] = null)));
  }
  function Iu(l, a, s) {
    return Un(
      l,
      s,
      a.replay,
      a.node,
      a.childIndex,
      a.blockedBoundary,
      a.hoistableState,
      a.abortSet,
      a.keyPath,
      a.formatContext,
      a.context,
      a.treeContext,
      a.row,
      a.componentStack
    );
  }
  function Mu(l, a, s) {
    var v = a.blockedSegment, x = Cr(
      l,
      v.chunks.length,
      null,
      a.formatContext,
      v.lastPushedText,
      !0
    );
    return v.children.push(x), v.lastPushedText = !1, ea(
      l,
      s,
      a.node,
      a.childIndex,
      a.blockedBoundary,
      x,
      a.blockedPreamble,
      a.hoistableState,
      a.abortSet,
      a.keyPath,
      a.formatContext,
      a.context,
      a.treeContext,
      a.row,
      a.componentStack
    );
  }
  function ut(l, a, s, v) {
    var x = a.formatContext, E = a.context, C = a.keyPath, _ = a.treeContext, m = a.componentStack, D = a.blockedSegment;
    if (D === null) {
      D = a.replay;
      try {
        return ar(l, a, s, v);
      } catch (se) {
        if (il(), s = se === ae ? ke() : se, l.status !== 12 && typeof s == "object" && s !== null) {
          if (typeof s.then == "function") {
            v = se === ae ? tr() : null, l = Iu(l, a, v).ping, s.then(l, l), a.formatContext = x, a.context = E, a.keyPath = C, a.treeContext = _, a.componentStack = m, a.replay = D, g(E);
            return;
          }
          if (s.message === "Maximum call stack size exceeded") {
            s = se === ae ? tr() : null, s = Iu(l, a, s), l.pingedTasks.push(s), a.formatContext = x, a.context = E, a.keyPath = C, a.treeContext = _, a.componentStack = m, a.replay = D, g(E);
            return;
          }
        }
      }
    } else {
      var U = D.children.length, Z = D.chunks.length;
      try {
        return ar(l, a, s, v);
      } catch (se) {
        if (il(), D.children.length = U, D.chunks.length = Z, s = se === ae ? ke() : se, l.status !== 12 && typeof s == "object" && s !== null) {
          if (typeof s.then == "function") {
            D = s, s = se === ae ? tr() : null, l = Mu(l, a, s).ping, D.then(l, l), a.formatContext = x, a.context = E, a.keyPath = C, a.treeContext = _, a.componentStack = m, g(E);
            return;
          }
          if (s.message === "Maximum call stack size exceeded") {
            D = se === ae ? tr() : null, D = Mu(l, a, D), l.pingedTasks.push(D), a.formatContext = x, a.context = E, a.keyPath = C, a.treeContext = _, a.componentStack = m, g(E);
            return;
          }
        }
      }
    }
    throw a.formatContext = x, a.context = E, a.keyPath = C, a.treeContext = _, g(E), s;
  }
  function Zc(l) {
    var a = l.blockedBoundary, s = l.blockedSegment;
    s !== null && (s.status = 3, ul(this, a, l.row, s));
  }
  function fc(l, a, s, v, x, E) {
    for (var C = 0; C < s.length; C++) {
      var _ = s[C];
      if (_.length === 4)
        fc(
          l,
          a,
          _[2],
          _[3],
          x,
          E
        );
      else {
        _ = _[5];
        var m = l, D = E, U = xo(
          m,
          null,
          /* @__PURE__ */ new Set(),
          null,
          null
        );
        U.parentFlushed = !0, U.rootSegmentID = _, U.status = 4, U.errorDigest = D, U.parentFlushed && m.clientRenderedBoundaries.push(U);
      }
    }
    if (s.length = 0, v !== null) {
      if (a === null) throw Error(G(487));
      if (a.status !== 4 && (a.status = 4, a.errorDigest = E, a.parentFlushed && l.clientRenderedBoundaries.push(a)), typeof v == "object") for (var Z in v) delete v[Z];
    }
  }
  function dc(l, a, s) {
    var v = l.blockedBoundary, x = l.blockedSegment;
    if (x !== null) {
      if (x.status === 6) return;
      x.status = 3;
    }
    var E = cl(l.componentStack);
    if (v === null) {
      if (a.status !== 13 && a.status !== 14) {
        if (v = l.replay, v === null) {
          a.trackedPostpones !== null && x !== null ? (v = a.trackedPostpones, Xt(a, s, E), Au(a, v, l, x), ul(a, null, l.row, x)) : (Xt(a, s, E), To(a, s));
          return;
        }
        v.pendingTasks--, v.pendingTasks === 0 && 0 < v.nodes.length && (x = Xt(a, s, E), fc(
          a,
          null,
          v.nodes,
          v.slots,
          s,
          x
        )), a.pendingRootTasks--, a.pendingRootTasks === 0 && Ou(a);
      }
    } else {
      var C = a.trackedPostpones;
      if (v.status !== 4) {
        if (C !== null && x !== null)
          return Xt(a, s, E), Au(a, C, l, x), v.fallbackAbortableTasks.forEach(function(_) {
            return dc(_, a, s);
          }), v.fallbackAbortableTasks.clear(), ul(a, v, l.row, x);
        v.status = 4, x = Xt(a, s, E), v.status = 4, v.errorDigest = x, Xc(a, v), v.parentFlushed && a.clientRenderedBoundaries.push(v);
      }
      v.pendingTasks--, x = v.row, x !== null && --x.pendingTasks === 0 && ir(a, x), v.fallbackAbortableTasks.forEach(function(_) {
        return dc(_, a, s);
      }), v.fallbackAbortableTasks.clear();
    }
    l = l.row, l !== null && --l.pendingTasks === 0 && ir(a, l), a.allPendingTasks--, a.allPendingTasks === 0 && Vc(a);
  }
  function Jc(l, a) {
    try {
      var s = l.renderState, v = s.onHeaders;
      if (v) {
        var x = s.headers;
        if (x) {
          s.headers = null;
          var E = x.preconnects;
          if (x.fontPreloads && (E && (E += ", "), E += x.fontPreloads), x.highImagePreloads && (E && (E += ", "), E += x.highImagePreloads), !a) {
            var C = s.styles.values(), _ = C.next();
            e: for (; 0 < x.remainingCapacity && !_.done; _ = C.next())
              for (var m = _.value.sheets.values(), D = m.next(); 0 < x.remainingCapacity && !D.done; D = m.next()) {
                var U = D.value, Z = U.props, se = Z.href, ce = U.props, Ge = Nc(ce.href, "style", {
                  crossOrigin: ce.crossOrigin,
                  integrity: ce.integrity,
                  nonce: ce.nonce,
                  type: ce.type,
                  fetchPriority: ce.fetchPriority,
                  referrerPolicy: ce.referrerPolicy,
                  media: ce.media
                });
                if (0 <= (x.remainingCapacity -= Ge.length + 2))
                  s.resets.style[se] = yt, E && (E += ", "), E += Ge, s.resets.style[se] = typeof Z.crossOrigin == "string" || typeof Z.integrity == "string" ? [Z.crossOrigin, Z.integrity] : yt;
                else break e;
              }
          }
          v(E ? { Link: E } : {});
        }
      }
    } catch (Se) {
      Xt(l, Se, {});
    }
  }
  function Ou(l) {
    l.trackedPostpones === null && Jc(l, !0), l.trackedPostpones === null && Ml(l), l.onShellError = Q, l = l.onShellReady, l();
  }
  function Vc(l) {
    Jc(
      l,
      l.trackedPostpones === null ? !0 : l.completedRootSegment === null || l.completedRootSegment.status !== 5
    ), Ml(l), l = l.onAllReady, l();
  }
  function Qc(l, a) {
    if (a.chunks.length === 0 && a.children.length === 1 && a.children[0].boundary === null && a.children[0].id === -1) {
      var s = a.children[0];
      s.id = a.id, s.parentFlushed = !0, s.status !== 1 && s.status !== 3 && s.status !== 4 || Qc(l, s);
    } else l.completedSegments.push(a);
  }
  function ta(l, a, s) {
    if (de !== null) {
      s = s.chunks;
      for (var v = 0, x = 0; x < s.length; x++)
        v += s[x].byteLength;
      a === null ? l.byteSize += v : a.byteSize += v;
    }
  }
  function ul(l, a, s, v) {
    if (s !== null && (--s.pendingTasks === 0 ? ir(l, s) : s.together && _a(l, s)), l.allPendingTasks--, a === null) {
      if (v !== null && v.parentFlushed) {
        if (l.completedRootSegment !== null)
          throw Error(G(389));
        l.completedRootSegment = v;
      }
      l.pendingRootTasks--, l.pendingRootTasks === 0 && Ou(l);
    } else if (a.pendingTasks--, a.status !== 4)
      if (a.pendingTasks === 0) {
        if (a.status === 0 && (a.status = 1), v !== null && v.parentFlushed && (v.status === 1 || v.status === 3) && Qc(a, v), a.parentFlushed && l.completedBoundaries.push(a), a.status === 1)
          s = a.row, s !== null && ma(s.hoistables, a.contentState), ct(l, a) || (a.fallbackAbortableTasks.forEach(Zc, l), a.fallbackAbortableTasks.clear(), s !== null && --s.pendingTasks === 0 && ir(l, s)), l.pendingRootTasks === 0 && l.trackedPostpones === null && a.contentPreamble !== null && Ml(l);
        else if (a.status === 5 && (a = a.row, a !== null)) {
          if (l.trackedPostpones !== null) {
            s = l.trackedPostpones;
            var x = a.next;
            if (x !== null && (v = x.boundaries, v !== null))
              for (x.boundaries = null, x = 0; x < v.length; x++) {
                var E = v[x];
                mu(l, s, E), ul(l, E, null, null);
              }
          }
          --a.pendingTasks === 0 && ir(l, a);
        }
      } else
        v === null || !v.parentFlushed || v.status !== 1 && v.status !== 3 || (Qc(a, v), a.completedSegments.length === 1 && a.parentFlushed && l.partialBoundaries.push(a)), a = a.row, a !== null && a.together && _a(l, a);
    l.allPendingTasks === 0 && Vc(l);
  }
  function ts(l) {
    if (l.status !== 14 && l.status !== 13) {
      var a = Wn, s = zi.H;
      zi.H = ji;
      var v = zi.A;
      zi.A = ic;
      var x = ze;
      ze = l;
      var E = rr;
      rr = l.resumableState;
      try {
        var C = l.pingedTasks, _;
        for (_ = 0; _ < C.length; _++) {
          var m = C[_], D = l, U = m.blockedSegment;
          if (U === null) {
            var Z = D;
            if (m.replay.pendingTasks !== 0) {
              g(m.context);
              try {
                if (typeof m.replay.slots == "number" ? La(
                  Z,
                  m,
                  m.replay.slots,
                  m.node,
                  m.childIndex
                ) : uc(Z, m), m.replay.pendingTasks === 1 && 0 < m.replay.nodes.length)
                  throw Error(G(488));
                m.replay.pendingTasks--, m.abortSet.delete(m), ul(
                  Z,
                  m.blockedBoundary,
                  m.row,
                  null
                );
              } catch (cr) {
                il();
                var se = cr === ae ? ke() : cr;
                if (typeof se == "object" && se !== null && typeof se.then == "function") {
                  var ce = m.ping;
                  se.then(ce, ce), m.thenableState = cr === ae ? tr() : null;
                } else {
                  m.replay.pendingTasks--, m.abortSet.delete(m);
                  var Ge = cl(m.componentStack);
                  D = void 0;
                  var Se = Z, Xe = m.blockedBoundary, Yn = Z.status === 12 ? Z.fatalError : se, Fn = m.replay.nodes, Zt = m.replay.slots;
                  D = Xt(
                    Se,
                    Yn,
                    Ge
                  ), fc(
                    Se,
                    Xe,
                    Fn,
                    Zt,
                    Yn,
                    D
                  ), Z.pendingRootTasks--, Z.pendingRootTasks === 0 && Ou(Z), Z.allPendingTasks--, Z.allPendingTasks === 0 && Vc(Z);
                }
              } finally {
              }
            }
          } else if (Z = void 0, Se = U, Se.status === 0) {
            Se.status = 6, g(m.context);
            var Dr = Se.children.length, fl = Se.chunks.length;
            try {
              uc(D, m), Se.lastPushedText && Se.textEmbedded && Se.chunks.push(Hl), m.abortSet.delete(m), Se.status = 1, ta(D, m.blockedBoundary, Se), ul(
                D,
                m.blockedBoundary,
                m.row,
                Se
              );
            } catch (cr) {
              il(), Se.children.length = Dr, Se.chunks.length = fl;
              var Pe = cr === ae ? ke() : D.status === 12 ? D.fatalError : cr;
              if (D.status === 12 && D.trackedPostpones !== null) {
                var or = D.trackedPostpones, tt = cl(m.componentStack);
                m.abortSet.delete(m), Xt(D, Pe, tt), Au(D, or, m, Se), ul(
                  D,
                  m.blockedBoundary,
                  m.row,
                  Se
                );
              } else if (typeof Pe == "object" && Pe !== null && typeof Pe.then == "function") {
                Se.status = 0, m.thenableState = cr === ae ? tr() : null;
                var kr = m.ping;
                Pe.then(kr, kr);
              } else {
                var Pi = cl(m.componentStack);
                m.abortSet.delete(m), Se.status = 4;
                var Gn = m.blockedBoundary, Na = m.row;
                if (Na !== null && --Na.pendingTasks === 0 && ir(D, Na), D.allPendingTasks--, Z = Xt(
                  D,
                  Pe,
                  Pi
                ), Gn === null) To(D, Pe);
                else if (Gn.pendingTasks--, Gn.status !== 4) {
                  Gn.status = 4, Gn.errorDigest = Z, Xc(D, Gn);
                  var dl = Gn.row;
                  dl !== null && --dl.pendingTasks === 0 && ir(D, dl), Gn.parentFlushed && D.clientRenderedBoundaries.push(Gn), D.pendingRootTasks === 0 && D.trackedPostpones === null && Gn.contentPreamble !== null && Ml(D);
                }
                D.allPendingTasks === 0 && Vc(D);
              }
            } finally {
            }
          }
        }
        C.splice(0, _), l.destination !== null && po(l, l.destination);
      } catch (cr) {
        Xt(l, cr, {}), To(l, cr);
      } finally {
        rr = E, zi.H = s, zi.A = v, s === ji && g(a), ze = x;
      }
    }
  }
  function za(l, a, s) {
    a.preambleChildren.length && s.push(a.preambleChildren);
    for (var v = !1, x = 0; x < a.children.length; x++)
      v = hc(
        l,
        a.children[x],
        s
      ) || v;
    return v;
  }
  function hc(l, a, s) {
    var v = a.boundary;
    if (v === null)
      return za(
        l,
        a,
        s
      );
    var x = v.contentPreamble, E = v.fallbackPreamble;
    if (x === null || E === null) return !1;
    switch (v.status) {
      case 1:
        if (Vo(l.renderState, x), l.byteSize += v.byteSize, a = v.completedSegments[0], !a) throw Error(G(391));
        return za(
          l,
          a,
          s
        );
      case 5:
        if (l.trackedPostpones !== null) return !0;
      case 4:
        if (a.status === 1)
          return Vo(l.renderState, E), za(
            l,
            a,
            s
          );
      default:
        return !0;
    }
  }
  function Ml(l) {
    if (l.completedRootSegment && l.completedPreambleSegments === null) {
      var a = [], s = l.byteSize, v = hc(
        l,
        l.completedRootSegment,
        a
      ), x = l.renderState.preamble;
      v === !1 || x.headChunks && x.bodyChunks ? l.completedPreambleSegments = a : l.byteSize = s;
    }
  }
  function wo(l, a, s, v) {
    switch (s.parentFlushed = !0, s.status) {
      case 0:
        s.id = l.nextSegmentId++;
      case 5:
        return v = s.id, s.lastPushedText = !1, s.textEmbedded = !1, l = l.renderState, V(a, xu), V(a, l.placeholderPrefix), l = le(v.toString(16)), V(a, l), ue(a, Qo);
      case 1:
        s.status = 2;
        var x = !0, E = s.chunks, C = 0;
        s = s.children;
        for (var _ = 0; _ < s.length; _++) {
          for (x = s[_]; C < x.index; C++)
            V(a, E[C]);
          x = gc(l, a, x, v);
        }
        for (; C < E.length - 1; C++)
          V(a, E[C]);
        return C < E.length && (x = ue(a, E[C])), x;
      case 3:
        return !0;
      default:
        throw Error(G(390));
    }
  }
  var ra = 0;
  function gc(l, a, s, v) {
    var x = s.boundary;
    if (x === null)
      return wo(l, a, s, v);
    if (x.parentFlushed = !0, x.status === 4) {
      var E = x.row;
      E !== null && --E.pendingTasks === 0 && ir(l, E), x = x.errorDigest, ue(a, St), V(a, Tu), x && (V(a, Sl), V(a, le(Fe(x))), V(
        a,
        fi
      )), ue(a, wu), wo(l, a, s, v);
    } else if (x.status !== 1)
      x.status === 0 && (x.rootSegmentID = l.nextSegmentId++), 0 < x.completedSegments.length && l.partialBoundaries.push(x), di(
        a,
        l.renderState,
        x.rootSegmentID
      ), v && ma(v, x.fallbackState), wo(l, a, s, v);
    else if (!Ol && ct(l, x) && (ra + x.byteSize > l.progressiveChunkSize || go(x.contentState)))
      x.rootSegmentID = l.nextSegmentId++, l.completedBoundaries.push(x), di(
        a,
        l.renderState,
        x.rootSegmentID
      ), wo(l, a, s, v);
    else {
      if (ra += x.byteSize, v && ma(v, x.contentState), s = x.row, s !== null && ct(l, x) && --s.pendingTasks === 0 && ir(l, s), ue(a, Hi), s = x.completedSegments, s.length !== 1) throw Error(G(391));
      gc(l, a, s[0], v);
    }
    return ue(a, Sn);
  }
  function la(l, a, s, v) {
    return Ra(
      a,
      l.renderState,
      s.parentFormatContext,
      s.id
    ), gc(l, a, s, v), Fl(a, s.parentFormatContext);
  }
  function rs(l, a, s) {
    ra = s.byteSize;
    for (var v = s.completedSegments, x = 0; x < v.length; x++)
      ls(
        l,
        a,
        s,
        v[x]
      );
    v.length = 0, v = s.row, v !== null && ct(l, s) && --v.pendingTasks === 0 && ir(l, v), jr(
      a,
      s.contentState,
      l.renderState
    ), v = l.resumableState, l = l.renderState, x = s.rootSegmentID, s = s.contentState;
    var E = l.stylesToHoist;
    return l.stylesToHoist = !1, V(a, l.startInlineScript), V(a, $e), E ? (!(v.instructions & 4) && (v.instructions |= 4, V(a, Xi)), !(v.instructions & 2) && (v.instructions |= 2, V(a, so)), v.instructions & 8 ? V(a, ju) : (v.instructions |= 8, V(a, Pt))) : (!(v.instructions & 2) && (v.instructions |= 2, V(a, so)), V(a, fo)), v = le(x.toString(16)), V(a, l.boundaryPrefix), V(a, v), V(a, Gi), V(a, l.segmentPrefix), V(a, v), E ? (V(a, qo), _t(a, s)) : V(a, _c), s = ue(a, xi), Tr(a, l) && s;
  }
  function ls(l, a, s, v) {
    if (v.status === 2) return !0;
    var x = s.contentState, E = v.id;
    if (E === -1) {
      if ((v.id = s.rootSegmentID) === -1)
        throw Error(G(392));
      return la(l, a, v, x);
    }
    return E === s.rootSegmentID ? la(l, a, v, x) : (la(l, a, v, x), s = l.resumableState, l = l.renderState, V(a, l.startInlineScript), V(a, $e), s.instructions & 1 ? V(a, Eu) : (s.instructions |= 1, V(a, pu)), V(a, l.segmentPrefix), E = le(E.toString(16)), V(a, E), V(a, Ru), V(a, l.placeholderPrefix), V(a, E), a = ue(a, uo), a);
  }
  var Ol = !1;
  function po(l, a) {
    Wt = new Uint8Array(2048), ge = 0;
    try {
      if (!(0 < l.pendingRootTasks)) {
        var s, v = l.completedRootSegment;
        if (v !== null) {
          if (v.status === 5) return;
          var x = l.completedPreambleSegments;
          if (x === null) return;
          ra = l.byteSize;
          var E = l.resumableState, C = l.renderState, _ = C.preamble, m = _.htmlChunks, D = _.headChunks, U;
          if (m) {
            for (U = 0; U < m.length; U++)
              V(a, m[U]);
            if (D)
              for (U = 0; U < D.length; U++)
                V(a, D[U]);
            else
              V(a, xt("head")), V(a, $e);
          } else if (D)
            for (U = 0; U < D.length; U++)
              V(a, D[U]);
          var Z = C.charsetChunks;
          for (U = 0; U < Z.length; U++)
            V(a, Z[U]);
          Z.length = 0, C.preconnects.forEach(Mn, a), C.preconnects.clear();
          var se = C.viewportChunks;
          for (U = 0; U < se.length; U++)
            V(a, se[U]);
          se.length = 0, C.fontPreloads.forEach(Mn, a), C.fontPreloads.clear(), C.highImagePreloads.forEach(Mn, a), C.highImagePreloads.clear(), Rl = C, C.styles.forEach(tc, a), Rl = null;
          var ce = C.importMapChunks;
          for (U = 0; U < ce.length; U++)
            V(a, ce[U]);
          ce.length = 0, C.bootstrapScripts.forEach(Mn, a), C.scripts.forEach(Mn, a), C.scripts.clear(), C.bulkPreloads.forEach(Mn, a), C.bulkPreloads.clear(), m || D || (E.instructions |= 32);
          var Ge = C.hoistableChunks;
          for (U = 0; U < Ge.length; U++)
            V(a, Ge[U]);
          for (E = Ge.length = 0; E < x.length; E++) {
            var Se = x[E];
            for (C = 0; C < Se.length; C++)
              gc(l, a, Se[C], null);
          }
          var Xe = l.renderState.preamble, Yn = Xe.headChunks;
          (Xe.htmlChunks || Yn) && V(a, Xl("head"));
          var Fn = Xe.bodyChunks;
          if (Fn)
            for (x = 0; x < Fn.length; x++)
              V(a, Fn[x]);
          gc(l, a, v, null), l.completedRootSegment = null;
          var Zt = l.renderState;
          if (l.allPendingTasks !== 0 || l.clientRenderedBoundaries.length !== 0 || l.completedBoundaries.length !== 0 || l.trackedPostpones !== null && (l.trackedPostpones.rootNodes.length !== 0 || l.trackedPostpones.rootSlots !== null)) {
            var Dr = l.resumableState;
            if (!(Dr.instructions & 64)) {
              if (Dr.instructions |= 64, V(a, Zt.startInlineScript), !(Dr.instructions & 32)) {
                Dr.instructions |= 32;
                var fl = "_" + Dr.idPrefix + "R_";
                V(a, en), V(
                  a,
                  le(Fe(fl))
                ), V(a, xn);
              }
              V(a, $e), V(a, kl), ue(a, qe);
            }
          }
          Tr(a, Zt);
        }
        var Pe = l.renderState;
        v = 0;
        var or = Pe.viewportChunks;
        for (v = 0; v < or.length; v++)
          V(a, or[v]);
        or.length = 0, Pe.preconnects.forEach(Mn, a), Pe.preconnects.clear(), Pe.fontPreloads.forEach(Mn, a), Pe.fontPreloads.clear(), Pe.highImagePreloads.forEach(
          Mn,
          a
        ), Pe.highImagePreloads.clear(), Pe.styles.forEach(On, a), Pe.scripts.forEach(Mn, a), Pe.scripts.clear(), Pe.bulkPreloads.forEach(Mn, a), Pe.bulkPreloads.clear();
        var tt = Pe.hoistableChunks;
        for (v = 0; v < tt.length; v++)
          V(a, tt[v]);
        tt.length = 0;
        var kr = l.clientRenderedBoundaries;
        for (s = 0; s < kr.length; s++) {
          var Pi = kr[s];
          Pe = a;
          var Gn = l.resumableState, Na = l.renderState, dl = Pi.rootSegmentID, cr = Pi.errorDigest;
          V(
            Pe,
            Na.startInlineScript
          ), V(Pe, $e), Gn.instructions & 4 ? V(Pe, ml) : (Gn.instructions |= 4, V(Pe, ho)), V(Pe, Na.boundaryPrefix), V(Pe, le(dl.toString(16))), V(Pe, Zi), cr && (V(
            Pe,
            Ji
          ), V(
            Pe,
            le(
              Ca(cr || "")
            )
          ));
          var _l = ue(
            Pe,
            Cu
          );
          if (!_l) {
            l.destination = null, s++, kr.splice(0, s);
            return;
          }
        }
        kr.splice(0, s);
        var ur = l.completedBoundaries;
        for (s = 0; s < ur.length; s++)
          if (!rs(l, a, ur[s])) {
            l.destination = null, s++, ur.splice(0, s);
            return;
          }
        ur.splice(0, s), Wl(a), Wt = new Uint8Array(2048), ge = 0, Ol = !0;
        var Wa = l.partialBoundaries;
        for (s = 0; s < Wa.length; s++) {
          var Ro = Wa[s];
          e: {
            kr = l, Pi = a, ra = Ro.byteSize;
            var Sr = Ro.completedSegments;
            for (_l = 0; _l < Sr.length; _l++)
              if (!ls(
                kr,
                Pi,
                Ro,
                Sr[_l]
              )) {
                _l++, Sr.splice(0, _l);
                var Co = !1;
                break e;
              }
            Sr.splice(0, _l);
            var sr = Ro.row;
            sr !== null && sr.together && Ro.pendingTasks === 1 && (sr.pendingTasks === 1 ? Fu(
              kr,
              sr,
              sr.hoistables
            ) : sr.pendingTasks--), Co = jr(
              Pi,
              Ro.contentState,
              kr.renderState
            );
          }
          if (!Co) {
            l.destination = null, s++, Wa.splice(0, s);
            return;
          }
        }
        Wa.splice(0, s), Ol = !1;
        var Ha = l.completedBoundaries;
        for (s = 0; s < Ha.length; s++)
          if (!rs(l, a, Ha[s])) {
            l.destination = null, s++, Ha.splice(0, s);
            return;
          }
        Ha.splice(0, s);
      }
    } finally {
      Ol = !1, l.allPendingTasks === 0 && l.clientRenderedBoundaries.length === 0 && l.completedBoundaries.length === 0 ? (l.flushScheduled = !1, s = l.resumableState, s.hasBody && V(a, Xl("body")), s.hasHtml && V(a, Xl("html")), Wl(a), l.status = 14, a.close(), l.destination = null) : Wl(a);
    }
  }
  function nl(l) {
    l.flushScheduled = l.destination !== null, Li(function() {
      return ts(l);
    }), bt(function() {
      l.status === 10 && (l.status = 11), l.trackedPostpones === null && Jc(l, l.pendingRootTasks === 0);
    });
  }
  function ia(l) {
    l.flushScheduled === !1 && l.pingedTasks.length === 0 && l.destination !== null && (l.flushScheduled = !0, bt(function() {
      var a = l.destination;
      a ? po(l, a) : l.flushScheduled = !1;
    }));
  }
  function Ba(l, a) {
    if (l.status === 13)
      l.status = 14, We(a, l.fatalError);
    else if (l.status !== 14 && l.destination === null) {
      l.destination = a;
      try {
        po(l, a);
      } catch (s) {
        Xt(l, s, {}), To(l, s);
      }
    }
  }
  function sl(l, a) {
    (l.status === 11 || l.status === 10) && (l.status = 12);
    try {
      var s = l.abortableTasks;
      if (0 < s.size) {
        var v = a === void 0 ? Error(G(432)) : typeof a == "object" && a !== null && typeof a.then == "function" ? Error(G(530)) : a;
        l.fatalError = v, s.forEach(function(x) {
          return dc(x, l, v);
        }), s.clear();
      }
      l.destination !== null && po(l, l.destination);
    } catch (x) {
      Xt(l, x, {}), To(l, x);
    }
  }
  function _u(l, a, s) {
    if (a === null) s.rootNodes.push(l);
    else {
      var v = s.workingMap, x = v.get(a);
      x === void 0 && (x = [a[1], a[2], [], null], v.set(a, x), _u(x, a[0], s)), x[2].push(l);
    }
  }
  function is(l) {
    var a = l.trackedPostpones;
    if (a === null || a.rootNodes.length === 0 && a.rootSlots === null)
      return l.trackedPostpones = null;
    if (l.completedRootSegment === null || l.completedRootSegment.status !== 5 && l.completedPreambleSegments !== null) {
      var s = l.nextSegmentId, v = a.rootSlots, x = l.resumableState;
      x.bootstrapScriptContent = void 0, x.bootstrapScripts = void 0, x.bootstrapModules = void 0;
    } else {
      s = 0, v = -1, x = l.resumableState;
      var E = l.renderState;
      x.nextFormID = 0, x.hasBody = !1, x.hasHtml = !1, x.unknownResources = { font: E.resets.font }, x.dnsResources = E.resets.dns, x.connectResources = E.resets.connect, x.imageResources = E.resets.image, x.styleResources = E.resets.style, x.scriptResources = {}, x.moduleUnknownResources = {}, x.moduleScriptResources = {}, x.instructions = 0;
    }
    return {
      nextSegmentId: s,
      rootFormatContext: l.rootFormatContext,
      progressiveChunkSize: l.progressiveChunkSize,
      resumableState: l.resumableState,
      replayNodes: a.rootNodes,
      replaySlots: v
    };
  }
  function Eo() {
    var l = Te.version;
    if (l !== "19.2.4")
      throw Error(
        G(
          527,
          l,
          "19.2.4"
        )
      );
  }
  return Eo(), Eo(), Ds.prerender = function(l, a) {
    return new Promise(function(s, v) {
      var x = a ? a.onHeaders : void 0, E;
      x && (E = function(U) {
        x(new Headers(U));
      });
      var C = Wo(
        a ? a.identifierPrefix : void 0,
        a ? a.unstable_externalRuntimeSrc : void 0,
        a ? a.bootstrapScriptContent : void 0,
        a ? a.bootstrapScripts : void 0,
        a ? a.bootstrapModules : void 0
      ), _ = Ma(
        l,
        C,
        ga(
          C,
          void 0,
          a ? a.unstable_externalRuntimeSrc : void 0,
          a ? a.importMap : void 0,
          E,
          a ? a.maxHeadersLength : void 0
        ),
        Fc(a ? a.namespaceURI : void 0),
        a ? a.progressiveChunkSize : void 0,
        a ? a.onError : void 0,
        function() {
          var U = new ReadableStream(
            {
              type: "bytes",
              pull: function(Z) {
                Ba(_, Z);
              },
              cancel: function(Z) {
                _.destination = null, sl(_, Z);
              }
            },
            { highWaterMark: 0 }
          );
          U = { postponed: is(_), prelude: U }, s(U);
        },
        void 0,
        void 0,
        v,
        a ? a.onPostpone : void 0
      );
      if (a && a.signal) {
        var m = a.signal;
        if (m.aborted) sl(_, m.reason);
        else {
          var D = function() {
            sl(_, m.reason), m.removeEventListener("abort", D);
          };
          m.addEventListener("abort", D);
        }
      }
      nl(_);
    });
  }, Ds.renderToReadableStream = function(l, a) {
    return new Promise(function(s, v) {
      var x, E, C = new Promise(function(ce, Ge) {
        E = ce, x = Ge;
      }), _ = a ? a.onHeaders : void 0, m;
      _ && (m = function(ce) {
        _(new Headers(ce));
      });
      var D = Wo(
        a ? a.identifierPrefix : void 0,
        a ? a.unstable_externalRuntimeSrc : void 0,
        a ? a.bootstrapScriptContent : void 0,
        a ? a.bootstrapScripts : void 0,
        a ? a.bootstrapModules : void 0
      ), U = yo(
        l,
        D,
        ga(
          D,
          a ? a.nonce : void 0,
          a ? a.unstable_externalRuntimeSrc : void 0,
          a ? a.importMap : void 0,
          m,
          a ? a.maxHeadersLength : void 0
        ),
        Fc(a ? a.namespaceURI : void 0),
        a ? a.progressiveChunkSize : void 0,
        a ? a.onError : void 0,
        E,
        function() {
          var ce = new ReadableStream(
            {
              type: "bytes",
              pull: function(Ge) {
                Ba(U, Ge);
              },
              cancel: function(Ge) {
                U.destination = null, sl(U, Ge);
              }
            },
            { highWaterMark: 0 }
          );
          ce.allReady = C, s(ce);
        },
        function(ce) {
          C.catch(function() {
          }), v(ce);
        },
        x,
        a ? a.onPostpone : void 0,
        a ? a.formState : void 0
      );
      if (a && a.signal) {
        var Z = a.signal;
        if (Z.aborted) sl(U, Z.reason);
        else {
          var se = function() {
            sl(U, Z.reason), Z.removeEventListener("abort", se);
          };
          Z.addEventListener("abort", se);
        }
      }
      nl(U);
    });
  }, Ds.resume = function(l, a, s) {
    return new Promise(function(v, x) {
      var E, C, _ = new Promise(function(Z, se) {
        C = Z, E = se;
      }), m = nt(
        l,
        a,
        ga(
          a.resumableState,
          s ? s.nonce : void 0,
          void 0,
          void 0,
          void 0,
          void 0
        ),
        s ? s.onError : void 0,
        C,
        function() {
          var Z = new ReadableStream(
            {
              type: "bytes",
              pull: function(se) {
                Ba(m, se);
              },
              cancel: function(se) {
                m.destination = null, sl(m, se);
              }
            },
            { highWaterMark: 0 }
          );
          Z.allReady = _, v(Z);
        },
        function(Z) {
          _.catch(function() {
          }), x(Z);
        },
        E,
        s ? s.onPostpone : void 0
      );
      if (s && s.signal) {
        var D = s.signal;
        if (D.aborted) sl(m, D.reason);
        else {
          var U = function() {
            sl(m, D.reason), D.removeEventListener("abort", U);
          };
          D.addEventListener("abort", U);
        }
      }
      nl(m);
    });
  }, Ds.resumeAndPrerender = function(l, a, s) {
    return new Promise(function(v, x) {
      var E = el(
        l,
        a,
        ga(
          a.resumableState,
          void 0,
          void 0,
          void 0,
          void 0,
          void 0
        ),
        s ? s.onError : void 0,
        function() {
          var m = new ReadableStream(
            {
              type: "bytes",
              pull: function(D) {
                Ba(E, D);
              },
              cancel: function(D) {
                E.destination = null, sl(E, D);
              }
            },
            { highWaterMark: 0 }
          );
          m = { postponed: is(E), prelude: m }, v(m);
        },
        void 0,
        void 0,
        x,
        s ? s.onPostpone : void 0
      );
      if (s && s.signal) {
        var C = s.signal;
        if (C.aborted) sl(E, C.reason);
        else {
          var _ = function() {
            sl(E, C.reason), C.removeEventListener("abort", _);
          };
          C.addEventListener("abort", _);
        }
      }
      nl(E);
    });
  }, Ds.version = "19.2.4", Ds;
}
var Vs = {};
/**
 * @license React
 * react-dom-server-legacy.browser.development.js
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
var Pf;
function Lf() {
  return Pf || (Pf = 1, process.env.NODE_ENV !== "production" && function() {
    function Te(n, r, u, h) {
      return "" + r + (u === "s" ? "\\73 " : "\\53 ") + h;
    }
    function Vn(n, r, u, h) {
      return "" + r + (u === "s" ? "\\u0073" : "\\u0053") + h;
    }
    function G(n) {
      return n === null || typeof n != "object" ? null : (n = ec && n[ec] || n["@@iterator"], typeof n == "function" ? n : null);
    }
    function vt(n) {
      return n = Object.prototype.toString.call(n), n.slice(8, n.length - 1);
    }
    function Qn(n) {
      var r = JSON.stringify(n);
      return '"' + n + '"' === r ? n : r;
    }
    function kn(n) {
      switch (typeof n) {
        case "string":
          return JSON.stringify(
            10 >= n.length ? n : n.slice(0, 10) + "..."
          );
        case "object":
          return Ti(n) ? "[...]" : n !== null && n.$$typeof === Rr ? "client" : (n = vt(n), n === "Object" ? "{...}" : n);
        case "function":
          return n.$$typeof === Rr ? "client" : (n = n.displayName || n.name) ? "function " + n : "function";
        default:
          return String(n);
      }
    }
    function Qe(n) {
      if (typeof n == "string") return n;
      switch (n) {
        case _r:
          return "Suspense";
        case qr:
          return "SuspenseList";
      }
      if (typeof n == "object")
        switch (n.$$typeof) {
          case Kr:
            return Qe(n.render);
          case Tn:
            return Qe(n.type);
          case jr:
            var r = n._payload;
            n = n._init;
            try {
              return Qe(n(r));
            } catch {
            }
        }
      return "";
    }
    function Kn(n, r) {
      var u = vt(n);
      if (u !== "Object" && u !== "Array") return u;
      var h = -1, y = 0;
      if (Ti(n))
        if (tc.has(n)) {
          var p = tc.get(n);
          u = "<" + Qe(p) + ">";
          for (var P = 0; P < n.length; P++) {
            var M = n[P];
            M = typeof M == "string" ? M : typeof M == "object" && M !== null ? "{" + Kn(M) + "}" : "{" + kn(M) + "}", "" + P === r ? (h = u.length, y = M.length, u += M) : u = 15 > M.length && 40 > u.length + M.length ? u + M : u + "{...}";
          }
          u += "</" + Qe(p) + ">";
        } else {
          for (u = "[", p = 0; p < n.length; p++)
            0 < p && (u += ", "), P = n[p], P = typeof P == "object" && P !== null ? Kn(P) : kn(P), "" + p === r ? (h = u.length, y = P.length, u += P) : u = 10 > P.length && 40 > u.length + P.length ? u + P : u + "...";
          u += "]";
        }
      else if (n.$$typeof === Ca)
        u = "<" + Qe(n.type) + "/>";
      else {
        if (n.$$typeof === Rr) return "client";
        if (nc.has(n)) {
          for (u = nc.get(n), u = "<" + (Qe(u) || "..."), p = Object.keys(n), P = 0; P < p.length; P++) {
            u += " ", M = p[P], u += Qn(M) + "=";
            var K = n[M], L = M === r && typeof K == "object" && K !== null ? Kn(K) : kn(K);
            typeof K != "string" && (L = "{" + L + "}"), M === r ? (h = u.length, y = L.length, u += L) : u = 10 > L.length && 40 > u.length + L.length ? u + L : u + "...";
          }
          u += ">";
        } else {
          for (u = "{", p = Object.keys(n), P = 0; P < p.length; P++)
            0 < P && (u += ", "), M = p[P], u += Qn(M) + ": ", K = n[M], K = typeof K == "object" && K !== null ? Kn(K) : kn(K), M === r ? (h = u.length, y = K.length, u += K) : u = 10 > K.length && 40 > u.length + K.length ? u + K : u + "...";
          u += "}";
        }
      }
      return r === void 0 ? u : -1 < h && 0 < y ? (n = " ".repeat(h) + "^".repeat(y), `
  ` + u + `
  ` + n) : `
  ` + u;
    }
    function Nl(n, r) {
      var u = n.length & 3, h = n.length - u, y = r;
      for (r = 0; r < h; ) {
        var p = n.charCodeAt(r) & 255 | (n.charCodeAt(++r) & 255) << 8 | (n.charCodeAt(++r) & 255) << 16 | (n.charCodeAt(++r) & 255) << 24;
        ++r, p = 3432918353 * (p & 65535) + ((3432918353 * (p >>> 16) & 65535) << 16) & 4294967295, p = p << 15 | p >>> 17, p = 461845907 * (p & 65535) + ((461845907 * (p >>> 16) & 65535) << 16) & 4294967295, y ^= p, y = y << 13 | y >>> 19, y = 5 * (y & 65535) + ((5 * (y >>> 16) & 65535) << 16) & 4294967295, y = (y & 65535) + 27492 + (((y >>> 16) + 58964 & 65535) << 16);
      }
      switch (p = 0, u) {
        case 3:
          p ^= (n.charCodeAt(r + 2) & 255) << 16;
        case 2:
          p ^= (n.charCodeAt(r + 1) & 255) << 8;
        case 1:
          p ^= n.charCodeAt(r) & 255, p = 3432918353 * (p & 65535) + ((3432918353 * (p >>> 16) & 65535) << 16) & 4294967295, p = p << 15 | p >>> 17, y ^= 461845907 * (p & 65535) + ((461845907 * (p >>> 16) & 65535) << 16) & 4294967295;
      }
      return y ^= n.length, y ^= y >>> 16, y = 2246822507 * (y & 65535) + ((2246822507 * (y >>> 16) & 65535) << 16) & 4294967295, y ^= y >>> 13, y = 3266489909 * (y & 65535) + ((3266489909 * (y >>> 16) & 65535) << 16) & 4294967295, (y ^ y >>> 16) >>> 0;
    }
    function Et(n) {
      return typeof Symbol == "function" && Symbol.toStringTag && n[Symbol.toStringTag] || n.constructor.name || "Object";
    }
    function jt(n) {
      try {
        return H(n), !1;
      } catch {
        return !0;
      }
    }
    function H(n) {
      return "" + n;
    }
    function pe(n, r) {
      if (jt(n))
        return console.error(
          "The provided `%s` attribute is an unsupported type %s. This value must be coerced to a string before using it here.",
          r,
          Et(n)
        ), H(n);
    }
    function Ar(n, r) {
      if (jt(n))
        return console.error(
          "The provided `%s` CSS property is an unsupported type %s. This value must be coerced to a string before using it here.",
          r,
          Et(n)
        ), H(n);
    }
    function ne(n) {
      if (jt(n))
        return console.error(
          "The provided HTML markup uses a value of unsupported type %s. This value must be coerced to a string before using it here.",
          Et(n)
        ), H(n);
    }
    function N(n) {
      return en.call(Sa, n) ? !0 : en.call(ka, n) ? !1 : Vi.test(n) ? Sa[n] = !0 : (ka[n] = !0, console.error("Invalid attribute name: `%s`", n), !1);
    }
    function Zr(n, r) {
      _t[r.type] || r.onChange || r.onInput || r.readOnly || r.disabled || r.value == null || console.error(
        n === "select" ? "You provided a `value` prop to a form field without an `onChange` handler. This will render a read-only field. If the field should be mutable use `defaultValue`. Otherwise, set `onChange`." : "You provided a `value` prop to a form field without an `onChange` handler. This will render a read-only field. If the field should be mutable use `defaultValue`. Otherwise, set either `onChange` or `readOnly`."
      ), r.onChange || r.readOnly || r.disabled || r.checked == null || console.error(
        "You provided a `checked` prop to a form field without an `onChange` handler. This will render a read-only field. If the field should be mutable use `defaultChecked`. Otherwise, set either `onChange` or `readOnly`."
      );
    }
    function zo(n, r) {
      if (en.call(Il, r) && Il[r])
        return !0;
      if (Su.test(r)) {
        if (n = "aria-" + r.slice(4).toLowerCase(), n = bs.hasOwnProperty(n) ? n : null, n == null)
          return console.error(
            "Invalid ARIA attribute `%s`. ARIA attributes follow the pattern aria-* and must be lowercase.",
            r
          ), Il[r] = !0;
        if (r !== n)
          return console.error(
            "Invalid ARIA attribute `%s`. Did you mean `%s`?",
            r,
            n
          ), Il[r] = !0;
      }
      if ($u.test(r)) {
        if (n = r.toLowerCase(), n = bs.hasOwnProperty(n) ? n : null, n == null) return Il[r] = !0, !1;
        r !== n && (console.error(
          "Unknown ARIA attribute `%s`. Did you mean `%s`?",
          r,
          n
        ), Il[r] = !0);
      }
      return !0;
    }
    function _i(n, r) {
      var u = [], h;
      for (h in r)
        zo(n, h) || u.push(h);
      r = u.map(function(y) {
        return "`" + y + "`";
      }).join(", "), u.length === 1 ? console.error(
        "Invalid aria prop %s on <%s> tag. For details, see https://react.dev/link/invalid-aria-props",
        r,
        n
      ) : 1 < u.length && console.error(
        "Invalid aria props %s on <%s> tag. For details, see https://react.dev/link/invalid-aria-props",
        r,
        n
      );
    }
    function Bo(n, r, u, h) {
      if (en.call(Yt, r) && Yt[r])
        return !0;
      var y = r.toLowerCase();
      if (y === "onfocusin" || y === "onfocusout")
        return console.error(
          "React uses onFocus and onBlur instead of onFocusIn and onFocusOut. All React events are normalized to bubble, so onFocusIn and onFocusOut are not needed/supported by React."
        ), Yt[r] = !0;
      if (typeof u == "function" && (n === "form" && r === "action" || n === "input" && r === "formAction" || n === "button" && r === "formAction"))
        return !0;
      if (wi.test(r))
        return es.test(r) && console.error(
          "Invalid event handler property `%s`. React events use the camelCase naming convention, for example `onClick`.",
          r
        ), Yt[r] = !0;
      if (Pa.test(r) || Nc.test(r)) return !0;
      if (y === "innerhtml")
        return console.error(
          "Directly setting property `innerHTML` is not permitted. For more information, lookup documentation on `dangerouslySetInnerHTML`."
        ), Yt[r] = !0;
      if (y === "aria")
        return console.error(
          "The `aria` attribute is reserved for future use in React. Pass individual `aria-` attributes instead."
        ), Yt[r] = !0;
      if (y === "is" && u !== null && u !== void 0 && typeof u != "string")
        return console.error(
          "Received a `%s` for a string attribute `is`. If this is expected, cast the value to a string.",
          typeof u
        ), Yt[r] = !0;
      if (typeof u == "number" && isNaN(u))
        return console.error(
          "Received NaN for the `%s` attribute. If this is expected, cast the value to a string.",
          r
        ), Yt[r] = !0;
      if (rc.hasOwnProperty(y)) {
        if (y = rc[y], y !== r)
          return console.error(
            "Invalid DOM property `%s`. Did you mean `%s`?",
            r,
            y
          ), Yt[r] = !0;
      } else if (r !== y)
        return console.error(
          "React does not recognize the `%s` prop on a DOM element. If you intentionally want it to appear in the DOM as a custom attribute, spell it as lowercase `%s` instead. If you accidentally passed it from a parent component, remove it from the DOM element.",
          r,
          y
        ), Yt[r] = !0;
      switch (r) {
        case "dangerouslySetInnerHTML":
        case "children":
        case "style":
        case "suppressContentEditableWarning":
        case "suppressHydrationWarning":
        case "defaultValue":
        case "defaultChecked":
        case "innerHTML":
        case "ref":
          return !0;
        case "innerText":
        case "textContent":
          return !0;
      }
      switch (typeof u) {
        case "boolean":
          switch (r) {
            case "autoFocus":
            case "checked":
            case "multiple":
            case "muted":
            case "selected":
            case "contentEditable":
            case "spellCheck":
            case "draggable":
            case "value":
            case "autoReverse":
            case "externalResourcesRequired":
            case "focusable":
            case "preserveAlpha":
            case "allowFullScreen":
            case "async":
            case "autoPlay":
            case "controls":
            case "default":
            case "defer":
            case "disabled":
            case "disablePictureInPicture":
            case "disableRemotePlayback":
            case "formNoValidate":
            case "hidden":
            case "loop":
            case "noModule":
            case "noValidate":
            case "open":
            case "playsInline":
            case "readOnly":
            case "required":
            case "reversed":
            case "scoped":
            case "seamless":
            case "itemScope":
            case "capture":
            case "download":
            case "inert":
              return !0;
            default:
              return y = r.toLowerCase().slice(0, 5), y === "data-" || y === "aria-" ? !0 : (u ? console.error(
                'Received `%s` for a non-boolean attribute `%s`.\n\nIf you want to write it to the DOM, pass a string instead: %s="%s" or %s={value.toString()}.',
                u,
                r,
                r,
                u,
                r
              ) : console.error(
                'Received `%s` for a non-boolean attribute `%s`.\n\nIf you want to write it to the DOM, pass a string instead: %s="%s" or %s={value.toString()}.\n\nIf you used to conditionally omit it with %s={condition && value}, pass %s={condition ? value : undefined} instead.',
                u,
                r,
                r,
                u,
                r,
                r,
                r
              ), Yt[r] = !0);
          }
        case "function":
        case "symbol":
          return Yt[r] = !0, !1;
        case "string":
          if (u === "false" || u === "true") {
            switch (r) {
              case "checked":
              case "selected":
              case "multiple":
              case "muted":
              case "allowFullScreen":
              case "async":
              case "autoPlay":
              case "controls":
              case "default":
              case "defer":
              case "disabled":
              case "disablePictureInPicture":
              case "disableRemotePlayback":
              case "formNoValidate":
              case "hidden":
              case "loop":
              case "noModule":
              case "noValidate":
              case "open":
              case "playsInline":
              case "readOnly":
              case "required":
              case "reversed":
              case "scoped":
              case "seamless":
              case "itemScope":
              case "inert":
                break;
              default:
                return !0;
            }
            console.error(
              "Received the string `%s` for the boolean attribute `%s`. %s Did you mean %s={%s}?",
              u,
              r,
              u === "false" ? "The browser will interpret it as a truthy value." : 'Although this works, it will not work as expected if you pass the string "false".',
              r,
              u
            ), Yt[r] = !0;
          }
      }
      return !0;
    }
    function Di(n, r, u) {
      var h = [], y;
      for (y in r)
        Bo(n, y, r[y]) || h.push(y);
      r = h.map(function(p) {
        return "`" + p + "`";
      }).join(", "), h.length === 1 ? console.error(
        "Invalid value for prop %s on <%s> tag. Either remove it from the element, or pass a string or number value to keep it in the DOM. For details, see https://react.dev/link/attribute-behavior ",
        r,
        n
      ) : 1 < h.length && console.error(
        "Invalid values for props %s on <%s> tag. Either remove them from the element, or pass a string or number value to keep them in the DOM. For details, see https://react.dev/link/attribute-behavior ",
        r,
        n
      );
    }
    function Nt(n) {
      return n.replace(Dt, function(r, u) {
        return u.toUpperCase();
      });
    }
    function Ee(n) {
      if (typeof n == "boolean" || typeof n == "number" || typeof n == "bigint")
        return "" + n;
      ne(n), n = "" + n;
      var r = Wc.exec(n);
      if (r) {
        var u = "", h, y = 0;
        for (h = r.index; h < n.length; h++) {
          switch (n.charCodeAt(h)) {
            case 34:
              r = "&quot;";
              break;
            case 38:
              r = "&amp;";
              break;
            case 39:
              r = "&#x27;";
              break;
            case 60:
              r = "&lt;";
              break;
            case 62:
              r = "&gt;";
              break;
            default:
              continue;
          }
          y !== h && (u += n.slice(y, h)), y = h + 1, u += r;
        }
        n = y !== h ? u + n.slice(y, h) : u;
      }
      return n;
    }
    function Ir(n) {
      return ys.test("" + n) ? "javascript:throw new Error('React has blocked a javascript: URL as a security precaution.')" : n;
    }
    function Rt(n) {
      return ne(n), ("" + n).replace(ae, Vn);
    }
    function dn(n, r, u, h, y) {
      return {
        idPrefix: n === void 0 ? "" : n,
        nextFormID: 0,
        streamingFormat: 0,
        bootstrapScriptContent: u,
        bootstrapScripts: h,
        bootstrapModules: y,
        instructions: o,
        hasBody: !1,
        hasHtml: !1,
        unknownResources: {},
        dnsResources: {},
        connectResources: { default: {}, anonymous: {}, credentials: {} },
        imageResources: {},
        styleResources: {},
        scriptResources: {},
        moduleUnknownResources: {},
        moduleScriptResources: {}
      };
    }
    function bt(n, r, u, h) {
      return {
        insertionMode: n,
        selectedValue: r,
        tagScope: u,
        viewTransition: h
      };
    }
    function kc(n, r, u) {
      var h = n.tagScope & -25;
      switch (r) {
        case "noscript":
          return bt(xe, null, h | 1, null);
        case "select":
          return bt(
            xe,
            u.value != null ? u.value : u.defaultValue,
            h,
            null
          );
        case "svg":
          return bt(Ce, null, h, null);
        case "picture":
          return bt(xe, null, h | 2, null);
        case "math":
          return bt(_n, null, h, null);
        case "foreignObject":
          return bt(xe, null, h, null);
        case "table":
          return bt(Le, null, h, null);
        case "thead":
        case "tbody":
        case "tfoot":
          return bt(
            Je,
            null,
            h,
            null
          );
        case "colgroup":
          return bt(
            Me,
            null,
            h,
            null
          );
        case "tr":
          return bt(
            on,
            null,
            h,
            null
          );
        case "head":
          if (n.insertionMode < xe)
            return bt(
              me,
              null,
              h,
              null
            );
          break;
        case "html":
          if (n.insertionMode === De)
            return bt(
              ke,
              null,
              h,
              null
            );
      }
      return n.insertionMode >= Le || n.insertionMode < xe ? bt(xe, null, h, null) : n.tagScope !== h ? bt(
        n.insertionMode,
        n.selectedValue,
        h,
        null
      ) : n;
    }
    function eo(n) {
      return n === null ? null : {
        update: n.update,
        enter: "none",
        exit: "none",
        share: n.update,
        name: n.autoName,
        autoName: n.autoName,
        nameIdx: 0
      };
    }
    function Li(n, r) {
      return r.tagScope & 32 && (n.instructions |= 128), bt(
        r.insertionMode,
        r.selectedValue,
        r.tagScope | 12,
        eo(r.viewTransition)
      );
    }
    function Wt(n, r) {
      n = eo(r.viewTransition);
      var u = r.tagScope | 16;
      return n !== null && n.share !== "none" && (u |= 64), bt(
        r.insertionMode,
        r.selectedValue,
        u,
        n
      );
    }
    function ge(n, r) {
      if (typeof r != "object")
        throw Error(
          "The `style` prop expects a mapping from style properties to values, not a string. For example, style={{marginRight: spacing + 'em'}} when using JSX."
        );
      var u = !0, h;
      for (h in r)
        if (en.call(r, h)) {
          var y = r[h];
          if (y != null && typeof y != "boolean" && y !== "") {
            if (h.indexOf("--") === 0) {
              var p = Ee(h);
              Ar(y, h), y = Ee(("" + y).trim());
            } else {
              p = h;
              var P = y;
              if (-1 < p.indexOf("-")) {
                var M = p;
                Pn.hasOwnProperty(M) && Pn[M] || (Pn[M] = !0, console.error(
                  "Unsupported style property %s. Did you mean %s?",
                  M,
                  Nt(M.replace(Gt, "ms-"))
                ));
              } else if (Fa.test(p))
                M = p, Pn.hasOwnProperty(M) && Pn[M] || (Pn[M] = !0, console.error(
                  "Unsupported vendor-prefixed style property %s. Did you mean %s?",
                  M,
                  M.charAt(0).toUpperCase() + M.slice(1)
                ));
              else if (Ki.test(P)) {
                M = p;
                var K = P;
                gn.hasOwnProperty(K) && gn[K] || (gn[K] = !0, console.error(
                  `Style property values shouldn't contain a semicolon. Try "%s: %s" instead.`,
                  M,
                  K.replace(
                    Ki,
                    ""
                  )
                ));
              }
              typeof P == "number" && (isNaN(P) ? ma || (ma = !0, console.error(
                "`NaN` is an invalid value for the `%s` css style property.",
                p
              )) : isFinite(P) || go || (go = !0, console.error(
                "`Infinity` is an invalid value for the `%s` css style property.",
                p
              ))), p = h, P = Hn.get(p), P !== void 0 || (P = Ee(
                p.replace(vo, "-$1").toLowerCase().replace(Pu, "-ms-")
              ), Hn.set(p, P)), p = P, typeof y == "number" ? y = y === 0 || Qi.has(h) ? "" + y : y + "px" : (Ar(y, h), y = Ee(
                ("" + y).trim()
              ));
            }
            u ? (u = !1, n.push(
              nn,
              p,
              nr,
              y
            )) : n.push(sn, p, nr, y);
          }
        }
      u || n.push(Ae);
    }
    function V(n, r, u) {
      u && typeof u != "function" && typeof u != "symbol" && n.push(Ke, r, rn);
    }
    function ue(n, r, u) {
      typeof u != "function" && typeof u != "symbol" && typeof u != "boolean" && n.push(
        Ke,
        r,
        tn,
        Ee(u),
        Ae
      );
    }
    function Wl(n, r) {
      this.push('<input type="hidden"'), no(n), ue(this, "name", r), ue(this, "value", n), this.push(qi);
    }
    function no(n) {
      if (typeof n != "string")
        throw Error(
          "File/Blob fields are not yet supported in progressive forms. Will fallback to client hydration."
        );
    }
    function le(n, r) {
      if (typeof r.$$FORM_ACTION == "function") {
        var u = n.nextFormID++;
        n = n.idPrefix + u;
        try {
          var h = r.$$FORM_ACTION(n);
          if (h) {
            var y = h.data;
            y != null && y.forEach(no);
          }
          return h;
        } catch (p) {
          if (typeof p == "object" && p !== null && typeof p.then == "function")
            throw p;
          console.error(
            `Failed to serialize an action for progressive enhancement:
%s`,
            p
          );
        }
      }
      return null;
    }
    function F(n, r, u, h, y, p, P, M) {
      var K = null;
      if (typeof h == "function") {
        M === null || Aa || (Aa = !0, console.error(
          'Cannot specify a "name" prop for a button that specifies a function as a formAction. React needs it to encode which action should be invoked. It will get overridden.'
        )), y === null && p === null || lc || (lc = !0, console.error(
          "Cannot specify a formEncType or formMethod for a button that specifies a function as a formAction. React provides those automatically. They will get overridden."
        )), P === null || Ia || (Ia = !0, console.error(
          "Cannot specify a formTarget for a button that specifies a function as a formAction. The function will always be executed in the same window."
        ));
        var L = le(r, h);
        L !== null ? (M = L.name, h = L.action || "", y = L.encType, p = L.method, P = L.target, K = L.data) : (n.push(
          Ke,
          "formAction",
          tn,
          Ft,
          Ae
        ), P = p = y = h = M = null, Ct(r, u));
      }
      return M != null && de(n, "name", M), h != null && de(n, "formAction", h), y != null && de(n, "formEncType", y), p != null && de(n, "formMethod", p), P != null && de(n, "formTarget", P), K;
    }
    function de(n, r, u) {
      switch (r) {
        case "className":
          ue(n, "class", u);
          break;
        case "tabIndex":
          ue(n, "tabindex", u);
          break;
        case "dir":
        case "role":
        case "viewBox":
        case "width":
        case "height":
          ue(n, r, u);
          break;
        case "style":
          ge(n, u);
          break;
        case "src":
        case "href":
          if (u === "") {
            console.error(
              r === "src" ? 'An empty string ("") was passed to the %s attribute. This may cause the browser to download the whole page again over the network. To fix this, either do not render the element at all or pass null to %s instead of an empty string.' : 'An empty string ("") was passed to the %s attribute. To fix this, either do not render the element at all or pass null to %s instead of an empty string.',
              r,
              r
            );
            break;
          }
        case "action":
        case "formAction":
          if (u == null || typeof u == "function" || typeof u == "symbol" || typeof u == "boolean")
            break;
          pe(u, r), u = Ir("" + u), n.push(
            Ke,
            r,
            tn,
            Ee(u),
            Ae
          );
          break;
        case "defaultValue":
        case "defaultChecked":
        case "innerHTML":
        case "suppressContentEditableWarning":
        case "suppressHydrationWarning":
        case "ref":
          break;
        case "autoFocus":
        case "multiple":
        case "muted":
          V(n, r.toLowerCase(), u);
          break;
        case "xlinkHref":
          if (typeof u == "function" || typeof u == "symbol" || typeof u == "boolean")
            break;
          pe(u, r), u = Ir("" + u), n.push(
            Ke,
            "xlink:href",
            tn,
            Ee(u),
            Ae
          );
          break;
        case "contentEditable":
        case "spellCheck":
        case "draggable":
        case "value":
        case "autoReverse":
        case "externalResourcesRequired":
        case "focusable":
        case "preserveAlpha":
          typeof u != "function" && typeof u != "symbol" && n.push(
            Ke,
            r,
            tn,
            Ee(u),
            Ae
          );
          break;
        case "inert":
          u !== "" || ve[r] || (ve[r] = !0, console.error(
            "Received an empty string for a boolean attribute `%s`. This will treat the attribute as if it were false. Either pass `false` to silence this warning, or pass `true` if you used an empty string in earlier versions of React to indicate this attribute is true.",
            r
          ));
        case "allowFullScreen":
        case "async":
        case "autoPlay":
        case "controls":
        case "default":
        case "defer":
        case "disabled":
        case "disablePictureInPicture":
        case "disableRemotePlayback":
        case "formNoValidate":
        case "hidden":
        case "loop":
        case "noModule":
        case "noValidate":
        case "open":
        case "playsInline":
        case "readOnly":
        case "required":
        case "reversed":
        case "scoped":
        case "seamless":
        case "itemScope":
          u && typeof u != "function" && typeof u != "symbol" && n.push(Ke, r, rn);
          break;
        case "capture":
        case "download":
          u === !0 ? n.push(Ke, r, rn) : u !== !1 && typeof u != "function" && typeof u != "symbol" && n.push(
            Ke,
            r,
            tn,
            Ee(u),
            Ae
          );
          break;
        case "cols":
        case "rows":
        case "size":
        case "span":
          typeof u != "function" && typeof u != "symbol" && !isNaN(u) && 1 <= u && n.push(
            Ke,
            r,
            tn,
            Ee(u),
            Ae
          );
          break;
        case "rowSpan":
        case "start":
          typeof u == "function" || typeof u == "symbol" || isNaN(u) || n.push(
            Ke,
            r,
            tn,
            Ee(u),
            Ae
          );
          break;
        case "xlinkActuate":
          ue(n, "xlink:actuate", u);
          break;
        case "xlinkArcrole":
          ue(n, "xlink:arcrole", u);
          break;
        case "xlinkRole":
          ue(n, "xlink:role", u);
          break;
        case "xlinkShow":
          ue(n, "xlink:show", u);
          break;
        case "xlinkTitle":
          ue(n, "xlink:title", u);
          break;
        case "xlinkType":
          ue(n, "xlink:type", u);
          break;
        case "xmlBase":
          ue(n, "xml:base", u);
          break;
        case "xmlLang":
          ue(n, "xml:lang", u);
          break;
        case "xmlSpace":
          ue(n, "xml:space", u);
          break;
        default:
          if ((!(2 < r.length) || r[0] !== "o" && r[0] !== "O" || r[1] !== "n" && r[1] !== "N") && (r = ku.get(r) || r, N(r))) {
            switch (typeof u) {
              case "function":
              case "symbol":
                return;
              case "boolean":
                var h = r.toLowerCase().slice(0, 5);
                if (h !== "data-" && h !== "aria-") return;
            }
            n.push(
              Ke,
              r,
              tn,
              Ee(u),
              Ae
            );
          }
      }
    }
    function We(n, r, u) {
      if (r != null) {
        if (u != null)
          throw Error(
            "Can only set one of `children` or `props.dangerouslySetInnerHTML`."
          );
        if (typeof r != "object" || !("__html" in r))
          throw Error(
            "`props.dangerouslySetInnerHTML` must be in the form `{__html: ...}`. Please visit https://react.dev/link/dangerously-set-inner-html for more information."
          );
        r = r.__html, r != null && (ne(r), n.push("" + r));
      }
    }
    function Ze(n, r) {
      var u = n[r];
      u != null && (u = Ti(u), n.multiple && !u ? console.error(
        "The `%s` prop supplied to <select> must be an array if `multiple` is true.",
        r
      ) : !n.multiple && u && console.error(
        "The `%s` prop supplied to <select> must be a scalar value if `multiple` is false.",
        r
      ));
    }
    function He(n) {
      var r = "";
      return Cu.Children.forEach(n, function(u) {
        u != null && (r += u, ol || typeof u == "string" || typeof u == "number" || typeof u == "bigint" || (ol = !0, console.error(
          "Cannot infer the option value of complex children. Pass a `value` prop or use a plain string as children to <option>."
        )));
      }), r;
    }
    function Ct(n, r) {
      if ((n.instructions & 16) === o) {
        n.instructions |= 16;
        var u = r.preamble, h = r.bootstrapChunks;
        (u.htmlChunks || u.headChunks) && h.length === 0 ? (h.push(r.startInlineScript), si(h, n), h.push(
          Be,
          Hc,
          Q
        )) : h.unshift(
          r.startInlineScript,
          Be,
          Hc,
          Q
        );
      }
    }
    function _e(n, r) {
      n.push(lt("link"));
      for (var u in r)
        if (en.call(r, u)) {
          var h = r[u];
          if (h != null)
            switch (u) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(
                  "link is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                );
              default:
                de(n, u, h);
            }
        }
      return n.push(qi), null;
    }
    function pl(n) {
      return ne(n), ("" + n).replace(bo, Te);
    }
    function Mr(n, r, u) {
      n.push(lt(u));
      for (var h in r)
        if (en.call(r, h)) {
          var y = r[h];
          if (y != null)
            switch (h) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(
                  u + " is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                );
              default:
                de(n, h, y);
            }
        }
      return n.push(qi), null;
    }
    function Mt(n, r) {
      n.push(lt("title"));
      var u = null, h = null, y;
      for (y in r)
        if (en.call(r, y)) {
          var p = r[y];
          if (p != null)
            switch (y) {
              case "children":
                u = p;
                break;
              case "dangerouslySetInnerHTML":
                h = p;
                break;
              default:
                de(n, y, p);
            }
        }
      return n.push(Be), r = Array.isArray(u) ? 2 > u.length ? u[0] : null : u, typeof r != "function" && typeof r != "symbol" && r !== null && r !== void 0 && n.push(Ee("" + r)), We(n, h, u), n.push(kt("title")), null;
    }
    function rt(n, r) {
      n.push(lt("script"));
      var u = null, h = null, y;
      for (y in r)
        if (en.call(r, y)) {
          var p = r[y];
          if (p != null)
            switch (y) {
              case "children":
                u = p;
                break;
              case "dangerouslySetInnerHTML":
                h = p;
                break;
              default:
                de(n, y, p);
            }
        }
      return n.push(Be), u != null && typeof u != "string" && (r = typeof u == "number" ? "a number for children" : Array.isArray(u) ? "an array for children" : "something unexpected for children", console.error(
        "A script element was rendered with %s. If script element has children it must be a single string. Consider using dangerouslySetInnerHTML or passing a plain string as children.",
        r
      )), We(n, h, u), typeof u == "string" && n.push(Rt(u)), n.push(kt("script")), null;
    }
    function Sc(n, r, u) {
      n.push(lt(u));
      var h = u = null, y;
      for (y in r)
        if (en.call(r, y)) {
          var p = r[y];
          if (p != null)
            switch (y) {
              case "children":
                u = p;
                break;
              case "dangerouslySetInnerHTML":
                h = p;
                break;
              default:
                de(n, y, p);
            }
        }
      return n.push(Be), We(n, h, u), u;
    }
    function Fe(n, r, u) {
      n.push(lt(u));
      var h = u = null, y;
      for (y in r)
        if (en.call(r, y)) {
          var p = r[y];
          if (p != null)
            switch (y) {
              case "children":
                u = p;
                break;
              case "dangerouslySetInnerHTML":
                h = p;
                break;
              default:
                de(n, y, p);
            }
        }
      return n.push(Be), We(n, h, u), typeof u == "string" ? (n.push(Ee(u)), null) : u;
    }
    function lt(n) {
      var r = ic.get(n);
      if (r === void 0) {
        if (!rr.test(n)) throw Error("Invalid tag: " + n);
        r = "<" + n, ic.set(n, r);
      }
      return r;
    }
    function un(n, r, u, h, y, p, P, M, K) {
      _i(r, u), r !== "input" && r !== "textarea" && r !== "select" || u == null || u.value !== null || Kl || (Kl = !0, r === "select" && u.multiple ? console.error(
        "`value` prop on `%s` should not be null. Consider using an empty array when `multiple` is set to `true` to clear the component or `undefined` for uncontrolled components.",
        r
      ) : console.error(
        "`value` prop on `%s` should not be null. Consider using an empty string to clear the component or `undefined` for uncontrolled components.",
        r
      ));
      e: if (r.indexOf("-") === -1) var L = !1;
      else
        switch (r) {
          case "annotation-xml":
          case "color-profile":
          case "font-face":
          case "font-face-src":
          case "font-face-uri":
          case "font-face-format":
          case "font-face-name":
          case "missing-glyph":
            L = !1;
            break e;
          default:
            L = !0;
        }
      switch (L || typeof u.is == "string" || Di(r, u), !u.suppressContentEditableWarning && u.contentEditable && u.children != null && console.error(
        "A component is `contentEditable` and contains `children` managed by React. It is now your responsibility to guarantee that none of those nodes are unexpectedly modified or duplicated. This is probably not intentional."
      ), M.insertionMode !== Ce && M.insertionMode !== _n && r.indexOf("-") === -1 && r.toLowerCase() !== r && console.error(
        "<%s /> is using incorrect casing. Use PascalCase for React components, or lowercase for HTML elements.",
        r
      ), r) {
        case "div":
        case "span":
        case "svg":
        case "path":
          break;
        case "a":
          n.push(lt("a"));
          var J = null, ie = null, fe;
          for (fe in u)
            if (en.call(u, fe)) {
              var oe = u[fe];
              if (oe != null)
                switch (fe) {
                  case "children":
                    J = oe;
                    break;
                  case "dangerouslySetInnerHTML":
                    ie = oe;
                    break;
                  case "href":
                    oe === "" ? ue(n, "href", "") : de(n, fe, oe);
                    break;
                  default:
                    de(n, fe, oe);
                }
            }
          if (n.push(Be), We(n, ie, J), typeof J == "string") {
            n.push(Ee(J));
            var $ = null;
          } else $ = J;
          return $;
        case "g":
        case "p":
        case "li":
          break;
        case "select":
          Zr("select", u), Ze(u, "value"), Ze(u, "defaultValue"), u.value === void 0 || u.defaultValue === void 0 || il || (console.error(
            "Select elements must be either controlled or uncontrolled (specify either the value prop, or the defaultValue prop, but not both). Decide between using a controlled or uncontrolled select element and remove one of these props. More info: https://react.dev/link/controlled-components"
          ), il = !0), n.push(lt("select"));
          var Ne = null, Cn = null, Re;
          for (Re in u)
            if (en.call(u, Re)) {
              var ln = u[Re];
              if (ln != null)
                switch (Re) {
                  case "children":
                    Ne = ln;
                    break;
                  case "dangerouslySetInnerHTML":
                    Cn = ln;
                    break;
                  case "defaultValue":
                  case "value":
                    break;
                  default:
                    de(
                      n,
                      Re,
                      ln
                    );
                }
            }
          return n.push(Be), We(n, Cn, Ne), Ne;
        case "option":
          var Jt = M.selectedValue;
          n.push(lt("option"));
          var Vt = null, cn = null, mt = null, ni = null, Pr;
          for (Pr in u)
            if (en.call(u, Pr)) {
              var he = u[Pr];
              if (he != null)
                switch (Pr) {
                  case "children":
                    Vt = he;
                    break;
                  case "selected":
                    mt = he, Ri || (console.error(
                      "Use the `defaultValue` or `value` props on <select> instead of setting `selected` on <option>."
                    ), Ri = !0);
                    break;
                  case "dangerouslySetInnerHTML":
                    ni = he;
                    break;
                  case "value":
                    cn = he;
                  default:
                    de(
                      n,
                      Pr,
                      he
                    );
                }
            }
          if (Jt != null) {
            if (cn !== null) {
              pe(cn, "value");
              var vn = "" + cn;
            } else
              ni === null || ql || (ql = !0, console.error(
                "Pass a `value` prop if you set dangerouslyInnerHTML so React knows which value should be selected."
              )), vn = He(Vt);
            if (Ti(Jt)) {
              for (var bn = 0; bn < Jt.length; bn++)
                if (pe(Jt[bn], "value"), "" + Jt[bn] === vn) {
                  n.push(' selected=""');
                  break;
                }
            } else
              pe(Jt, "select.value"), "" + Jt === vn && n.push(' selected=""');
          } else mt && n.push(' selected=""');
          return n.push(Be), We(n, ni, Vt), Vt;
        case "textarea":
          Zr("textarea", u), u.value === void 0 || u.defaultValue === void 0 || al || (console.error(
            "Textarea elements must be either controlled or uncontrolled (specify either the value prop, or the defaultValue prop, but not both). Decide between using a controlled or uncontrolled textarea and remove one of these props. More info: https://react.dev/link/controlled-components"
          ), al = !0), n.push(lt("textarea"));
          var wn = null, Xn = null, Ve = null, Oe;
          for (Oe in u)
            if (en.call(u, Oe)) {
              var dr = u[Oe];
              if (dr != null)
                switch (Oe) {
                  case "children":
                    Ve = dr;
                    break;
                  case "value":
                    wn = dr;
                    break;
                  case "defaultValue":
                    Xn = dr;
                    break;
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "`dangerouslySetInnerHTML` does not make sense on <textarea>."
                    );
                  default:
                    de(
                      n,
                      Oe,
                      dr
                    );
                }
            }
          if (wn === null && Xn !== null && (wn = Xn), n.push(Be), Ve != null) {
            if (console.error(
              "Use the `defaultValue` or `value` props instead of setting children on <textarea>."
            ), wn != null)
              throw Error(
                "If you supply `defaultValue` on a <textarea>, do not pass children."
              );
            if (Ti(Ve)) {
              if (1 < Ve.length)
                throw Error("<textarea> can only have at most one child.");
              ne(Ve[0]), wn = "" + Ve[0];
            }
            ne(Ve), wn = "" + Ve;
          }
          return typeof wn == "string" && wn[0] === `
` && n.push(ji), wn !== null && (pe(wn, "value"), n.push(Ee("" + wn))), null;
        case "input":
          Zr("input", u), n.push(lt("input"));
          var yn = null, Qt = null, Ln = null, zr = null, Ao = null, Br = null, ti = null, ri = null, li = null, zt;
          for (zt in u)
            if (en.call(u, zt)) {
              var e = u[zt];
              if (e != null)
                switch (zt) {
                  case "children":
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "input is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                    );
                  case "name":
                    yn = e;
                    break;
                  case "formAction":
                    Qt = e;
                    break;
                  case "formEncType":
                    Ln = e;
                    break;
                  case "formMethod":
                    zr = e;
                    break;
                  case "formTarget":
                    Ao = e;
                    break;
                  case "defaultChecked":
                    li = e;
                    break;
                  case "defaultValue":
                    ti = e;
                    break;
                  case "checked":
                    ri = e;
                    break;
                  case "value":
                    Br = e;
                    break;
                  default:
                    de(
                      n,
                      zt,
                      e
                    );
                }
            }
          Qt === null || u.type === "image" || u.type === "submit" || wt || (wt = !0, console.error(
            'An input can only specify a formAction along with type="submit" or type="image".'
          ));
          var t = F(
            n,
            h,
            y,
            Qt,
            Ln,
            zr,
            Ao,
            yn
          );
          return ri === null || li === null || tr || (console.error(
            "%s contains an input of type %s with both checked and defaultChecked props. Input elements must be either controlled or uncontrolled (specify either the checked prop, or the defaultChecked prop, but not both). Decide between using a controlled or uncontrolled input element and remove one of these props. More info: https://react.dev/link/controlled-components",
            "A component",
            u.type
          ), tr = !0), Br === null || ti === null || Ei || (console.error(
            "%s contains an input of type %s with both value and defaultValue props. Input elements must be either controlled or uncontrolled (specify either the value prop, or the defaultValue prop, but not both). Decide between using a controlled or uncontrolled input element and remove one of these props. More info: https://react.dev/link/controlled-components",
            "A component",
            u.type
          ), Ei = !0), ri !== null ? V(n, "checked", ri) : li !== null && V(n, "checked", li), Br !== null ? de(n, "value", Br) : ti !== null && de(n, "value", ti), n.push(qi), t != null && t.forEach(Wl, n), null;
        case "button":
          n.push(lt("button"));
          var c = null, d = null, b = null, w = null, k = null, A = null, X = null, O;
          for (O in u)
            if (en.call(u, O)) {
              var z = u[O];
              if (z != null)
                switch (O) {
                  case "children":
                    c = z;
                    break;
                  case "dangerouslySetInnerHTML":
                    d = z;
                    break;
                  case "name":
                    b = z;
                    break;
                  case "formAction":
                    w = z;
                    break;
                  case "formEncType":
                    k = z;
                    break;
                  case "formMethod":
                    A = z;
                    break;
                  case "formTarget":
                    X = z;
                    break;
                  default:
                    de(
                      n,
                      O,
                      z
                    );
                }
            }
          w === null || u.type == null || u.type === "submit" || wt || (wt = !0, console.error(
            'A button can only specify a formAction along with type="submit" or no type.'
          ));
          var ee = F(
            n,
            h,
            y,
            w,
            k,
            A,
            X,
            b
          );
          if (n.push(Be), ee != null && ee.forEach(Wl, n), We(n, d, c), typeof c == "string") {
            n.push(Ee(c));
            var re = null;
          } else re = c;
          return re;
        case "form":
          n.push(lt("form"));
          var te = null, j = null, we = null, pn = null, ye = null, Ue = null, jn;
          for (jn in u)
            if (en.call(u, jn)) {
              var ft = u[jn];
              if (ft != null)
                switch (jn) {
                  case "children":
                    te = ft;
                    break;
                  case "dangerouslySetInnerHTML":
                    j = ft;
                    break;
                  case "action":
                    we = ft;
                    break;
                  case "encType":
                    pn = ft;
                    break;
                  case "method":
                    ye = ft;
                    break;
                  case "target":
                    Ue = ft;
                    break;
                  default:
                    de(
                      n,
                      jn,
                      ft
                    );
                }
            }
          var an = null, be = null;
          if (typeof we == "function") {
            pn === null && ye === null || lc || (lc = !0, console.error(
              "Cannot specify a encType or method for a form that specifies a function as the action. React provides those automatically. They will get overridden."
            )), Ue === null || Ia || (Ia = !0, console.error(
              "Cannot specify a target for a form that specifies a function as the action. The function will always be executed in the same window."
            ));
            var dt = le(
              h,
              we
            );
            dt !== null ? (we = dt.action || "", pn = dt.encType, ye = dt.method, Ue = dt.target, an = dt.data, be = dt.name) : (n.push(
              Ke,
              "action",
              tn,
              Ft,
              Ae
            ), Ue = ye = pn = we = null, Ct(h, y));
          }
          if (we != null && de(n, "action", we), pn != null && de(n, "encType", pn), ye != null && de(n, "method", ye), Ue != null && de(n, "target", Ue), n.push(Be), be !== null && (n.push('<input type="hidden"'), ue(n, "name", be), n.push(qi), an != null && an.forEach(
            Wl,
            n
          )), We(n, j, te), typeof te == "string") {
            n.push(Ee(te));
            var hr = null;
          } else hr = te;
          return hr;
        case "menuitem":
          n.push(lt("menuitem"));
          for (var En in u)
            if (en.call(u, En)) {
              var zn = u[En];
              if (zn != null)
                switch (En) {
                  case "children":
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "menuitems cannot have `children` nor `dangerouslySetInnerHTML`."
                    );
                  default:
                    de(
                      n,
                      En,
                      zn
                    );
                }
            }
          return n.push(Be), null;
        case "object":
          n.push(lt("object"));
          var Nr = null, Kt = null, At;
          for (At in u)
            if (en.call(u, At)) {
              var Bn = u[At];
              if (Bn != null)
                switch (At) {
                  case "children":
                    Nr = Bn;
                    break;
                  case "dangerouslySetInnerHTML":
                    Kt = Bn;
                    break;
                  case "data":
                    pe(Bn, "data");
                    var pt = Ir("" + Bn);
                    if (pt === "") {
                      console.error(
                        'An empty string ("") was passed to the %s attribute. To fix this, either do not render the element at all or pass null to %s instead of an empty string.',
                        At,
                        At
                      );
                      break;
                    }
                    n.push(
                      Ke,
                      "data",
                      tn,
                      Ee(pt),
                      Ae
                    );
                    break;
                  default:
                    de(
                      n,
                      At,
                      Bn
                    );
                }
            }
          if (n.push(Be), We(n, Kt, Nr), typeof Nr == "string") {
            n.push(Ee(Nr));
            var Wr = null;
          } else Wr = Nr;
          return Wr;
        case "title":
          var rl = M.tagScope & 1, It = M.tagScope & 4;
          if (en.call(u, "children")) {
            var Zn = u.children, gr = Array.isArray(Zn) ? 2 > Zn.length ? Zn[0] : null : Zn;
            Array.isArray(Zn) && 1 < Zn.length ? console.error(
              "React expects the `children` prop of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found an Array with length %s instead. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert `children` of <title> tags to a single string value which is why Arrays of length greater than 1 are not supported. When using JSX it can be common to combine text nodes and value nodes. For example: <title>hello {nameOfUser}</title>. While not immediately apparent, `children` in this case is an Array with length 2. If your `children` prop is using this form try rewriting it using a template string: <title>{`hello ${nameOfUser}`}</title>.",
              Zn.length
            ) : typeof gr == "function" || typeof gr == "symbol" ? console.error(
              "React expect children of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found %s instead. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert children of <title> tags to a single string value.",
              typeof gr == "function" ? "a Function" : "a Sybmol"
            ) : gr && gr.toString === {}.toString && (gr.$$typeof != null ? console.error(
              "React expects the `children` prop of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found an object that appears to be a React element which never implements a suitable `toString` method. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert children of <title> tags to a single string value which is why rendering React elements is not supported. If the `children` of <title> is a React Component try moving the <title> tag into that component. If the `children` of <title> is some HTML markup change it to be Text only to be valid HTML."
            ) : console.error(
              "React expects the `children` prop of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found an object that does not implement a suitable `toString` method. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert children of <title> tags to a single string value. Using the default `toString` method available on every object is almost certainly an error. Consider whether the `children` of this <title> is an object in error and change it to a string or number value if so. Otherwise implement a `toString` method that React can use to produce a valid <title>."
            ));
          }
          if (M.insertionMode === Ce || rl || u.itemProp != null)
            var Hr = Mt(
              n,
              u
            );
          else
            It ? Hr = null : (Mt(y.hoistableChunks, u), Hr = void 0);
          return Hr;
        case "link":
          var vr = M.tagScope & 1, Ii = M.tagScope & 4, Ur = u.rel, ht = u.href, vl = u.precedence;
          if (M.insertionMode === Ce || vr || u.itemProp != null || typeof Ur != "string" || typeof ht != "string" || ht === "") {
            Ur === "stylesheet" && typeof u.precedence == "string" && (typeof ht == "string" && ht || console.error(
              'React encountered a `<link rel="stylesheet" .../>` with a `precedence` prop and expected the `href` prop to be a non-empty string but ecountered %s instead. If your intent was to have React hoist and deduplciate this stylesheet using the `precedence` prop ensure there is a non-empty string `href` prop as well, otherwise remove the `precedence` prop.',
              ht === null ? "`null`" : ht === void 0 ? "`undefined`" : ht === "" ? "an empty string" : 'something with type "' + typeof ht + '"'
            )), _e(n, u);
            var Bt = null;
          } else if (u.rel === "stylesheet")
            if (typeof vl != "string" || u.disabled != null || u.onLoad || u.onError) {
              if (typeof vl == "string") {
                if (u.disabled != null)
                  console.error(
                    'React encountered a `<link rel="stylesheet" .../>` with a `precedence` prop and a `disabled` prop. The presence of the `disabled` prop indicates an intent to manage the stylesheet active state from your from your Component code and React will not hoist or deduplicate this stylesheet. If your intent was to have React hoist and deduplciate this stylesheet using the `precedence` prop remove the `disabled` prop, otherwise remove the `precedence` prop.'
                  );
                else if (u.onLoad || u.onError) {
                  var Bu = u.onLoad && u.onError ? "`onLoad` and `onError` props" : u.onLoad ? "`onLoad` prop" : "`onError` prop";
                  console.error(
                    'React encountered a `<link rel="stylesheet" .../>` with a `precedence` prop and %s. The presence of loading and error handlers indicates an intent to manage the stylesheet loading state from your from your Component code and React will not hoist or deduplicate this stylesheet. If your intent was to have React hoist and deduplciate this stylesheet using the `precedence` prop remove the %s, otherwise remove the `precedence` prop.',
                    Bu,
                    Bu
                  );
                }
              }
              Bt = _e(
                n,
                u
              );
            } else {
              var ii = y.styles.get(vl), $n = h.styleResources.hasOwnProperty(
                ht
              ) ? h.styleResources[ht] : void 0;
              if ($n !== I) {
                h.styleResources[ht] = I, ii || (ii = {
                  precedence: Ee(vl),
                  rules: [],
                  hrefs: [],
                  sheets: /* @__PURE__ */ new Map()
                }, y.styles.set(vl, ii));
                var bl = {
                  state: x,
                  props: On({}, u, {
                    "data-precedence": u.precedence,
                    precedence: null
                  })
                };
                if ($n) {
                  $n.length === 2 && ga(bl.props, $n);
                  var br = y.preloads.stylesheets.get(ht);
                  br && 0 < br.length ? br.length = 0 : bl.state = E;
                }
                ii.sheets.set(ht, bl), P && P.stylesheets.add(bl);
              } else if (ii) {
                var xc = ii.sheets.get(ht);
                xc && P && P.stylesheets.add(xc);
              }
              K && n.push("<!-- -->"), Bt = null;
            }
          else
            u.onLoad || u.onError ? Bt = _e(
              n,
              u
            ) : (K && n.push("<!-- -->"), Bt = Ii ? null : _e(y.hoistableChunks, u));
          return Bt;
        case "script":
          var Io = M.tagScope & 1, zl = u.async;
          if (typeof u.src != "string" || !u.src || !zl || typeof zl == "function" || typeof zl == "symbol" || u.onLoad || u.onError || M.insertionMode === Ce || Io || u.itemProp != null)
            var Mo = rt(
              n,
              u
            );
          else {
            var Yr = u.src;
            if (u.type === "module")
              var Tc = h.moduleScriptResources, Nu = y.preloads.moduleScripts;
            else
              Tc = h.scriptResources, Nu = y.preloads.scripts;
            var yl = Tc.hasOwnProperty(Yr) ? Tc[Yr] : void 0;
            if (yl !== I) {
              Tc[Yr] = I;
              var wc = u;
              if (yl) {
                yl.length === 2 && (wc = On({}, u), ga(wc, yl));
                var yr = Nu.get(Yr);
                yr && (yr.length = 0);
              }
              var Oo = [];
              y.scripts.add(Oo), rt(Oo, wc);
            }
            K && n.push("<!-- -->"), Mo = null;
          }
          return Mo;
        case "style":
          var Va = M.tagScope & 1;
          if (en.call(u, "children")) {
            var ua = u.children, xl = Array.isArray(ua) ? 2 > ua.length ? ua[0] : null : ua;
            (typeof xl == "function" || typeof xl == "symbol" || Array.isArray(xl)) && console.error(
              "React expect children of <style> tags to be a string, number, or object with a `toString` method but found %s instead. In browsers style Elements can only have `Text` Nodes as children.",
              typeof xl == "function" ? "a Function" : typeof xl == "symbol" ? "a Sybmol" : "an Array"
            );
          }
          var Tl = u.precedence, ai = u.href, Gr = u.nonce;
          if (M.insertionMode === Ce || Va || u.itemProp != null || typeof Tl != "string" || typeof ai != "string" || ai === "") {
            n.push(lt("style"));
            var qt = null, sa = null, oi;
            for (oi in u)
              if (en.call(u, oi)) {
                var Qa = u[oi];
                if (Qa != null)
                  switch (oi) {
                    case "children":
                      qt = Qa;
                      break;
                    case "dangerouslySetInnerHTML":
                      sa = Qa;
                      break;
                    default:
                      de(
                        n,
                        oi,
                        Qa
                      );
                  }
              }
            n.push(Be);
            var Mi = Array.isArray(qt) ? 2 > qt.length ? qt[0] : null : qt;
            typeof Mi != "function" && typeof Mi != "symbol" && Mi !== null && Mi !== void 0 && n.push(pl(Mi)), We(
              n,
              sa,
              qt
            ), n.push(kt("style"));
            var ds = null;
          } else {
            ai.includes(" ") && console.error(
              'React expected the `href` prop for a <style> tag opting into hoisting semantics using the `precedence` prop to not have any spaces but ecountered spaces instead. using spaces in this prop will cause hydration of this style to fail on the client. The href for the <style> where this ocurred is "%s".',
              ai
            );
            var Fr = y.styles.get(Tl), Bl = h.styleResources.hasOwnProperty(ai) ? h.styleResources[ai] : void 0;
            if (Bl !== I) {
              h.styleResources[ai] = I, Bl && console.error(
                'React encountered a hoistable style tag for the same href as a preload: "%s". When using a style tag to inline styles you should not also preload it as a stylsheet.',
                ai
              ), Fr || (Fr = {
                precedence: Ee(Tl),
                rules: [],
                hrefs: [],
                sheets: /* @__PURE__ */ new Map()
              }, y.styles.set(
                Tl,
                Fr
              ));
              var pc = y.nonce.style;
              if (pc && pc !== Gr)
                console.error(
                  'React encountered a style tag with `precedence` "%s" and `nonce` "%s". When React manages style rules using `precedence` it will only include rules if the nonce matches the style nonce "%s" that was included with this render.',
                  Tl,
                  Gr,
                  pc
                );
              else {
                !pc && Gr && console.error(
                  'React encountered a style tag with `precedence` "%s" and `nonce` "%s". When React manages style rules using `precedence` it will only include a nonce attributes if you also provide the same style nonce value as a render option.',
                  Tl,
                  Gr
                ), Fr.hrefs.push(
                  Ee(ai)
                );
                var au = Fr.rules, ou = null, zs = null, Ka;
                for (Ka in u)
                  if (en.call(u, Ka)) {
                    var _o = u[Ka];
                    if (_o != null)
                      switch (Ka) {
                        case "children":
                          ou = _o;
                          break;
                        case "dangerouslySetInnerHTML":
                          zs = _o;
                      }
                  }
                var qa = Array.isArray(ou) ? 2 > ou.length ? ou[0] : null : ou;
                typeof qa != "function" && typeof qa != "symbol" && qa !== null && qa !== void 0 && au.push(pl(qa)), We(au, zs, ou);
              }
            }
            Fr && P && P.styles.add(Fr), K && n.push("<!-- -->"), ds = void 0;
          }
          return ds;
        case "meta":
          var cu = M.tagScope & 1, ms = M.tagScope & 4;
          if (M.insertionMode === Ce || cu || u.itemProp != null)
            var ws = Mr(
              n,
              u,
              "meta"
            );
          else
            K && n.push("<!-- -->"), ws = ms ? null : typeof u.charSet == "string" ? Mr(y.charsetChunks, u, "meta") : u.name === "viewport" ? Mr(y.viewportChunks, u, "meta") : Mr(
              y.hoistableChunks,
              u,
              "meta"
            );
          return ws;
        case "listing":
        case "pre":
          n.push(lt(r));
          var mr = null, ci = null, ja;
          for (ja in u)
            if (en.call(u, ja)) {
              var Wu = u[ja];
              if (Wu != null)
                switch (ja) {
                  case "children":
                    mr = Wu;
                    break;
                  case "dangerouslySetInnerHTML":
                    ci = Wu;
                    break;
                  default:
                    de(
                      n,
                      ja,
                      Wu
                    );
                }
            }
          if (n.push(Be), ci != null) {
            if (mr != null)
              throw Error(
                "Can only set one of `children` or `props.dangerouslySetInnerHTML`."
              );
            if (typeof ci != "object" || !("__html" in ci))
              throw Error(
                "`props.dangerouslySetInnerHTML` must be in the form `{__html: ...}`. Please visit https://react.dev/link/dangerously-set-inner-html for more information."
              );
            var wl = ci.__html;
            wl != null && (typeof wl == "string" && 0 < wl.length && wl[0] === `
` ? n.push(ji, wl) : (ne(wl), n.push("" + wl)));
          }
          return typeof mr == "string" && mr[0] === `
` && n.push(ji), mr;
        case "img":
          var gt = M.tagScope & 3, Jn = u.src, mn = u.srcSet;
          if (!(u.loading === "lazy" || !Jn && !mn || typeof Jn != "string" && Jn != null || typeof mn != "string" && mn != null || u.fetchPriority === "low" || gt) && (typeof Jn != "string" || Jn[4] !== ":" || Jn[0] !== "d" && Jn[0] !== "D" || Jn[1] !== "a" && Jn[1] !== "A" || Jn[2] !== "t" && Jn[2] !== "T" || Jn[3] !== "a" && Jn[3] !== "A") && (typeof mn != "string" || mn[4] !== ":" || mn[0] !== "d" && mn[0] !== "D" || mn[1] !== "a" && mn[1] !== "A" || mn[2] !== "t" && mn[2] !== "T" || mn[3] !== "a" && mn[3] !== "A")) {
            P !== null && M.tagScope & 64 && (P.suspenseyImages = !0);
            var ps = typeof u.sizes == "string" ? u.sizes : void 0, Do = mn ? mn + `
` + (ps || "") : Jn, Hu = y.preloads.images, Lo = Hu.get(Do);
            if (Lo)
              (u.fetchPriority === "high" || 10 > y.highImagePreloads.size) && (Hu.delete(Do), y.highImagePreloads.add(Lo));
            else if (!h.imageResources.hasOwnProperty(Do)) {
              h.imageResources[Do] = W;
              var Ec = u.crossOrigin, Uu = typeof Ec == "string" ? Ec === "use-credentials" ? Ec : "" : void 0, Rc = y.headers, uu;
              Rc && 0 < Rc.remainingCapacity && typeof u.srcSet != "string" && (u.fetchPriority === "high" || 500 > Rc.highImagePreloads.length) && (uu = Wo(Jn, "image", {
                imageSrcSet: u.srcSet,
                imageSizes: u.sizes,
                crossOrigin: Uu,
                integrity: u.integrity,
                nonce: u.nonce,
                type: u.type,
                fetchPriority: u.fetchPriority,
                referrerPolicy: u.refererPolicy
              }), 0 <= (Rc.remainingCapacity -= uu.length + 2)) ? (y.resets.image[Do] = W, Rc.highImagePreloads && (Rc.highImagePreloads += ", "), Rc.highImagePreloads += uu) : (Lo = [], _e(Lo, {
                rel: "preload",
                as: "image",
                href: mn ? void 0 : Jn,
                imageSrcSet: mn,
                imageSizes: ps,
                crossOrigin: Uu,
                integrity: u.integrity,
                type: u.type,
                fetchPriority: u.fetchPriority,
                referrerPolicy: u.referrerPolicy
              }), u.fetchPriority === "high" || 10 > y.highImagePreloads.size ? y.highImagePreloads.add(Lo) : (y.bulkPreloads.add(Lo), Hu.set(Do, Lo)));
            }
          }
          return Mr(n, u, "img");
        case "base":
        case "area":
        case "br":
        case "col":
        case "embed":
        case "hr":
        case "keygen":
        case "param":
        case "source":
        case "track":
        case "wbr":
          return Mr(n, u, r);
        case "annotation-xml":
        case "color-profile":
        case "font-face":
        case "font-face-src":
        case "font-face-uri":
        case "font-face-format":
        case "font-face-name":
        case "missing-glyph":
          break;
        case "head":
          if (M.insertionMode < xe) {
            var su = p || y.preamble;
            if (su.headChunks)
              throw Error("The `<head>` tag may only be rendered once.");
            p !== null && n.push("<!--head-->"), su.headChunks = [];
            var hs = Sc(
              su.headChunks,
              u,
              "head"
            );
          } else
            hs = Fe(
              n,
              u,
              "head"
            );
          return hs;
        case "body":
          if (M.insertionMode < xe) {
            var Es = p || y.preamble;
            if (Es.bodyChunks)
              throw Error("The `<body>` tag may only be rendered once.");
            p !== null && n.push("<!--body-->"), Es.bodyChunks = [];
            var As = Sc(
              Es.bodyChunks,
              u,
              "body"
            );
          } else
            As = Fe(
              n,
              u,
              "body"
            );
          return As;
        case "html":
          if (M.insertionMode === De) {
            var Yu = p || y.preamble;
            if (Yu.htmlChunks)
              throw Error("The `<html>` tag may only be rendered once.");
            p !== null && n.push("<!--html-->"), Yu.htmlChunks = [U];
            var Gu = Sc(
              Yu.htmlChunks,
              u,
              "html"
            );
          } else
            Gu = Fe(
              n,
              u,
              "html"
            );
          return Gu;
        default:
          if (r.indexOf("-") !== -1) {
            n.push(lt(r));
            var Cc = null, $a = null, fa;
            for (fa in u)
              if (en.call(u, fa)) {
                var Xr = u[fa];
                if (Xr != null) {
                  var fu = fa;
                  switch (fa) {
                    case "children":
                      Cc = Xr;
                      break;
                    case "dangerouslySetInnerHTML":
                      $a = Xr;
                      break;
                    case "style":
                      ge(n, Xr);
                      break;
                    case "suppressContentEditableWarning":
                    case "suppressHydrationWarning":
                    case "ref":
                      break;
                    case "className":
                      fu = "class";
                    default:
                      if (N(fa) && typeof Xr != "function" && typeof Xr != "symbol" && Xr !== !1) {
                        if (Xr === !0)
                          Xr = "";
                        else if (typeof Xr == "object")
                          continue;
                        n.push(
                          Ke,
                          fu,
                          tn,
                          Ee(Xr),
                          Ae
                        );
                      }
                  }
                }
              }
            return n.push(Be), We(
              n,
              $a,
              Cc
            ), Cc;
          }
      }
      return Fe(n, u, r);
    }
    function kt(n) {
      var r = lr.get(n);
      return r === void 0 && (r = "</" + n + ">", lr.set(n, r)), r;
    }
    function da(n, r) {
      n = n.preamble, n.htmlChunks === null && r.htmlChunks && (n.htmlChunks = r.htmlChunks), n.headChunks === null && r.headChunks && (n.headChunks = r.headChunks), n.bodyChunks === null && r.bodyChunks && (n.bodyChunks = r.bodyChunks);
    }
    function zi(n, r) {
      r = r.bootstrapChunks;
      for (var u = 0; u < r.length - 1; u++)
        n.push(r[u]);
      return u < r.length ? (u = r[u], r.length = 0, n.push(u)) : !0;
    }
    function Ot(n, r, u) {
      if (n.push(Uc), u === null)
        throw Error(
          "An ID must have been assigned before we can complete the boundary."
        );
      return n.push(r.boundaryPrefix), r = u.toString(16), n.push(r), n.push(ct);
    }
    function xr(n, r, u, h) {
      switch (u.insertionMode) {
        case De:
        case ke:
        case me:
        case xe:
          return n.push(ea), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(Un);
        case Ce:
          return n.push(na), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(Oa);
        case _n:
          return n.push(Xt), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(To);
        case Le:
          return n.push(Fu), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(_a);
        case Je:
          return n.push(Yc), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(Gc);
        case on:
          return n.push(Da), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(La);
        case Me:
          return n.push(uc), n.push(r.segmentPrefix), r = h.toString(16), n.push(r), n.push(sc);
        default:
          throw Error("Unknown insertion mode. This is a bug in React.");
      }
    }
    function El(n, r) {
      switch (r.insertionMode) {
        case De:
        case ke:
        case me:
        case xe:
          return n.push(Cr);
        case Ce:
          return n.push(cl);
        case _n:
          return n.push(ir);
        case Le:
          return n.push(cc);
        case Je:
          return n.push(jl);
        case on:
          return n.push(ar);
        case Me:
          return n.push(mu);
        default:
          throw Error("Unknown insertion mode. This is a bug in React.");
      }
    }
    function yt(n) {
      return JSON.stringify(n).replace(
        wo,
        function(r) {
          switch (r) {
            case "<":
              return "\\u003c";
            case "\u2028":
              return "\\u2028";
            case "\u2029":
              return "\\u2029";
            default:
              throw Error(
                "escapeJSStringsForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
              );
          }
        }
      );
    }
    function Rl(n) {
      return JSON.stringify(n).replace(
        ra,
        function(r) {
          switch (r) {
            case "&":
              return "\\u0026";
            case ">":
              return "\\u003e";
            case "<":
              return "\\u003c";
            case "\u2028":
              return "\\u2028";
            case "\u2029":
              return "\\u2029";
            default:
              throw Error(
                "escapeJSObjectForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
              );
          }
        }
      );
    }
    function Pc(n) {
      var r = n.rules, u = n.hrefs;
      0 < r.length && u.length === 0 && console.error(
        "React expected to have at least one href for an a hoistable style but found none. This is a bug in React."
      );
      var h = 0;
      if (u.length) {
        for (this.push(q.startInlineStyle), this.push(gc), this.push(n.precedence), this.push(la); h < u.length - 1; h++)
          this.push(u[h]), this.push(sl);
        for (this.push(u[h]), this.push(rs), h = 0; h < r.length; h++) this.push(r[h]);
        po = this.push(ls), Ol = !0, r.length = 0, u.length = 0;
      }
    }
    function qe(n) {
      return n.state !== C ? Ol = !0 : !1;
    }
    function $t(n, r, u) {
      return Ol = !1, po = !0, q = u, r.styles.forEach(Pc, n), q = null, r.stylesheets.forEach(qe), Ol && (u.stylesToHoist = !0), po;
    }
    function Rn(n) {
      for (var r = 0; r < n.length; r++) this.push(n[r]);
      n.length = 0;
    }
    function Xu(n) {
      _e(nl, n.props);
      for (var r = 0; r < nl.length; r++)
        this.push(nl[r]);
      nl.length = 0, n.state = C;
    }
    function et(n) {
      var r = 0 < n.sheets.size;
      n.sheets.forEach(Xu, this), n.sheets.clear();
      var u = n.rules, h = n.hrefs;
      if (!r || h.length) {
        if (this.push(q.startInlineStyle), this.push(ia), this.push(n.precedence), n = 0, h.length) {
          for (this.push(Ba); n < h.length - 1; n++)
            this.push(h[n]), this.push(sl);
          this.push(h[n]);
        }
        for (this.push(_u), n = 0; n < u.length; n++)
          this.push(u[n]);
        this.push(is), u.length = 0, h.length = 0;
      }
    }
    function hu(n) {
      if (n.state === x) {
        n.state = E;
        var r = n.props;
        for (_e(nl, {
          rel: "preload",
          as: "style",
          href: n.props.href,
          crossOrigin: r.crossOrigin,
          fetchPriority: r.fetchPriority,
          integrity: r.integrity,
          media: r.media,
          hrefLang: r.hrefLang,
          referrerPolicy: r.referrerPolicy
        }), n = 0; n < nl.length; n++)
          this.push(nl[n]);
        nl.length = 0;
      }
    }
    function gu(n) {
      n.sheets.forEach(hu, this), n.sheets.clear();
    }
    function si(n, r) {
      (r.instructions & S) === o && (r.instructions |= S, n.push(
        Eo,
        Ee("_" + r.idPrefix + "R_"),
        Ae
      ));
    }
    function vu(n, r) {
      n.push(l);
      var u = l;
      r.stylesheets.forEach(function(h) {
        if (h.state !== C)
          if (h.state === _)
            n.push(u), h = h.props.href, pe(h, "href"), h = Rl("" + h), n.push(h), n.push(v), u = a;
          else {
            n.push(u);
            var y = h.props["data-precedence"], p = h.props, P = Ir("" + h.props.href);
            P = Rl(P), n.push(P), pe(y, "precedence"), y = "" + y, n.push(s), y = Rl(y), n.push(y);
            for (var M in p)
              if (en.call(p, M) && (y = p[M], y != null))
                switch (M) {
                  case "href":
                  case "rel":
                  case "precedence":
                  case "data-precedence":
                    break;
                  case "children":
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "link is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                    );
                  default:
                    No(
                      n,
                      M,
                      y
                    );
                }
            n.push(v), u = a, h.state = _;
          }
      }), n.push(v);
    }
    function No(n, r, u) {
      var h = r.toLowerCase();
      switch (typeof u) {
        case "function":
        case "symbol":
          return;
      }
      switch (r) {
        case "innerHTML":
        case "dangerouslySetInnerHTML":
        case "suppressContentEditableWarning":
        case "suppressHydrationWarning":
        case "style":
        case "ref":
          return;
        case "className":
          h = "class", pe(u, h), r = "" + u;
          break;
        case "hidden":
          if (u === !1) return;
          r = "";
          break;
        case "src":
        case "href":
          u = Ir(u), pe(u, h), r = "" + u;
          break;
        default:
          if (2 < r.length && (r[0] === "o" || r[0] === "O") && (r[1] === "n" || r[1] === "N") || !N(r))
            return;
          pe(u, h), r = "" + u;
      }
      n.push(s), h = Rl(h), n.push(h), n.push(s), h = Rl(r), n.push(h);
    }
    function ha() {
      return { styles: /* @__PURE__ */ new Set(), stylesheets: /* @__PURE__ */ new Set(), suspenseyImages: !1 };
    }
    function Bi(n, r, u, h) {
      (n.scriptResources.hasOwnProperty(u) || n.moduleScriptResources.hasOwnProperty(u)) && console.error(
        'Internal React Error: React expected bootstrap script or module with src "%s" to not have been preloaded already. please file an issue',
        u
      ), n.scriptResources[u] = I, n.moduleScriptResources[u] = I, n = [], _e(n, h), r.bootstrapScripts.add(n);
    }
    function ga(n, r) {
      n.crossOrigin == null && (n.crossOrigin = r[0]), n.integrity == null && (n.integrity = r[1]);
    }
    function Wo(n, r, u) {
      n = Nn(n), r = Fc(r, "as"), r = "<" + n + '>; rel=preload; as="' + r + '"';
      for (var h in u)
        en.call(u, h) && (n = u[h], typeof n == "string" && (r += "; " + h.toLowerCase() + '="' + Fc(
          n,
          h
        ) + '"'));
      return r;
    }
    function Nn(n) {
      return pe(n, "href"), ("" + n).replace(
        m,
        An
      );
    }
    function An(n) {
      switch (n) {
        case "<":
          return "%3C";
        case ">":
          return "%3E";
        case `
`:
          return "%0A";
        case "\r":
          return "%0D";
        default:
          throw Error(
            "escapeLinkHrefForHeaderContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
          );
      }
    }
    function Fc(n, r) {
      return jt(n) && (console.error(
        "The provided `%s` option is an unsupported type %s. This value must be coerced to a string before using it here.",
        r,
        Et(n)
      ), H(n)), ("" + n).replace(
        D,
        va
      );
    }
    function va(n) {
      switch (n) {
        case '"':
          return "%22";
        case "'":
          return "%27";
        case ";":
          return "%3B";
        case ",":
          return "%2C";
        case `
`:
          return "%0A";
        case "\r":
          return "%0D";
        default:
          throw Error(
            "escapeStringForLinkHeaderQuotedParamValueContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
          );
      }
    }
    function Zu(n) {
      this.styles.add(n);
    }
    function bu(n) {
      this.stylesheets.add(n);
    }
    function ll(n, r) {
      r.styles.forEach(Zu, n), r.stylesheets.forEach(bu, n), r.suspenseyImages && (n.suspenseyImages = !0);
    }
    function Hl(n, r) {
      var u = n.idPrefix, h = [], y = n.bootstrapScriptContent, p = n.bootstrapScripts, P = n.bootstrapModules;
      if (y !== void 0 && (h.push("<script"), si(h, n), h.push(
        Be,
        Rt(y),
        Q
      )), u = {
        placeholderPrefix: u + "P:",
        segmentPrefix: u + "S:",
        boundaryPrefix: u + "B:",
        startInlineScript: "<script",
        startInlineStyle: "<style",
        preamble: { htmlChunks: null, headChunks: null, bodyChunks: null },
        externalRuntimeScript: null,
        bootstrapChunks: h,
        importMapChunks: [],
        onHeaders: void 0,
        headers: null,
        resets: {
          font: {},
          dns: {},
          connect: { default: {}, anonymous: {}, credentials: {} },
          image: {},
          style: {}
        },
        charsetChunks: [],
        viewportChunks: [],
        hoistableChunks: [],
        preconnects: /* @__PURE__ */ new Set(),
        fontPreloads: /* @__PURE__ */ new Set(),
        highImagePreloads: /* @__PURE__ */ new Set(),
        styles: /* @__PURE__ */ new Map(),
        bootstrapScripts: /* @__PURE__ */ new Set(),
        scripts: /* @__PURE__ */ new Set(),
        bulkPreloads: /* @__PURE__ */ new Set(),
        preloads: {
          images: /* @__PURE__ */ new Map(),
          stylesheets: /* @__PURE__ */ new Map(),
          scripts: /* @__PURE__ */ new Map(),
          moduleScripts: /* @__PURE__ */ new Map()
        },
        nonce: { script: void 0, style: void 0 },
        hoistableState: null,
        stylesToHoist: !1
      }, p !== void 0)
        for (y = 0; y < p.length; y++) {
          var M = p[y], K, L = void 0, J = void 0, ie = {
            rel: "preload",
            as: "script",
            fetchPriority: "low",
            nonce: void 0
          };
          typeof M == "string" ? ie.href = K = M : (ie.href = K = M.src, ie.integrity = J = typeof M.integrity == "string" ? M.integrity : void 0, ie.crossOrigin = L = typeof M == "string" || M.crossOrigin == null ? void 0 : M.crossOrigin === "use-credentials" ? "use-credentials" : ""), Bi(n, u, K, ie), h.push(
            '<script src="',
            Ee(K),
            Ae
          ), typeof J == "string" && h.push(
            ' integrity="',
            Ee(J),
            Ae
          ), typeof L == "string" && h.push(
            ' crossorigin="',
            Ee(L),
            Ae
          ), si(h, n), h.push(' async=""><\/script>');
        }
      if (P !== void 0)
        for (p = 0; p < P.length; p++)
          y = P[p], L = K = void 0, J = {
            rel: "modulepreload",
            fetchPriority: "low",
            nonce: void 0
          }, typeof y == "string" ? J.href = M = y : (J.href = M = y.src, J.integrity = L = typeof y.integrity == "string" ? y.integrity : void 0, J.crossOrigin = K = typeof y == "string" || y.crossOrigin == null ? void 0 : y.crossOrigin === "use-credentials" ? "use-credentials" : ""), Bi(
            n,
            u,
            M,
            J
          ), h.push(
            '<script type="module" src="',
            Ee(M),
            Ae
          ), typeof L == "string" && h.push(
            ' integrity="',
            Ee(L),
            Ae
          ), typeof K == "string" && h.push(
            ' crossorigin="',
            Ee(K),
            Ae
          ), si(h, n), h.push(' async=""><\/script>');
      return {
        placeholderPrefix: u.placeholderPrefix,
        segmentPrefix: u.segmentPrefix,
        boundaryPrefix: u.boundaryPrefix,
        startInlineScript: u.startInlineScript,
        startInlineStyle: u.startInlineStyle,
        preamble: u.preamble,
        externalRuntimeScript: u.externalRuntimeScript,
        bootstrapChunks: u.bootstrapChunks,
        importMapChunks: u.importMapChunks,
        onHeaders: u.onHeaders,
        headers: u.headers,
        resets: u.resets,
        charsetChunks: u.charsetChunks,
        viewportChunks: u.viewportChunks,
        hoistableChunks: u.hoistableChunks,
        preconnects: u.preconnects,
        fontPreloads: u.fontPreloads,
        highImagePreloads: u.highImagePreloads,
        styles: u.styles,
        bootstrapScripts: u.bootstrapScripts,
        scripts: u.scripts,
        bulkPreloads: u.bulkPreloads,
        preloads: u.preloads,
        nonce: u.nonce,
        stylesToHoist: u.stylesToHoist,
        generateStaticMarkup: r
      };
    }
    function Ju(n, r, u, h) {
      return u.generateStaticMarkup ? (n.push(Ee(r)), !1) : (r === "" ? n = h : (h && n.push("<!-- -->"), n.push(Ee(r)), n = !0), n);
    }
    function ba(n, r, u, h) {
      r.generateStaticMarkup || u && h && n.push("<!-- -->");
    }
    function qn(n) {
      if (n == null) return null;
      if (typeof n == "function")
        return n.$$typeof === se ? null : n.displayName || n.name || null;
      if (typeof n == "string") return n;
      switch (n) {
        case Ql:
          return "Fragment";
        case $o:
          return "Profiler";
        case jo:
          return "StrictMode";
        case _r:
          return "Suspense";
        case qr:
          return "SuspenseList";
        case Al:
          return "Activity";
      }
      if (typeof n == "object")
        switch (typeof n.tag == "number" && console.error(
          "Received an unexpected object in getComponentNameFromType(). This is likely a bug in React. Please file an issue."
        ), n.$$typeof) {
          case ot:
            return "Portal";
          case Qr:
            return n.displayName || "Context";
          case Dc:
            return (n._context.displayName || "Context") + ".Consumer";
          case Kr:
            var r = n.render;
            return n = n.displayName, n || (n = r.displayName || r.name || "", n = n !== "" ? "ForwardRef(" + n + ")" : "ForwardRef"), n;
          case Tn:
            return r = n.displayName || null, r !== null ? r : qn(n.type) || "Memo";
          case jr:
            r = n._payload, n = n._init;
            try {
              return qn(n(r));
            } catch {
            }
        }
      return null;
    }
    function Ni(n, r) {
      if (n !== r) {
        n.context._currentValue2 = n.parentValue, n = n.parent;
        var u = r.parent;
        if (n === null) {
          if (u !== null)
            throw Error(
              "The stacks must reach the root at the same time. This is a bug in React."
            );
        } else {
          if (u === null)
            throw Error(
              "The stacks must reach the root at the same time. This is a bug in React."
            );
          Ni(n, u);
        }
        r.context._currentValue2 = r.value;
      }
    }
    function Wi(n) {
      n.context._currentValue2 = n.parentValue, n = n.parent, n !== null && Wi(n);
    }
    function ya(n) {
      var r = n.parent;
      r !== null && ya(r), n.context._currentValue2 = n.value;
    }
    function Ht(n, r) {
      if (n.context._currentValue2 = n.parentValue, n = n.parent, n === null)
        throw Error(
          "The depth must equal at least at zero before reaching the root. This is a bug in React."
        );
      n.depth === r.depth ? Ni(n, r) : Ht(n, r);
    }
    function Or(n, r) {
      var u = r.parent;
      if (u === null)
        throw Error(
          "The depth must equal at least at zero before reaching the root. This is a bug in React."
        );
      n.depth === u.depth ? Ni(n, u) : Or(n, u), r.context._currentValue2 = r.value;
    }
    function xn(n) {
      var r = Se;
      r !== n && (r === null ? ya(n) : n === null ? Wi(r) : r.depth === n.depth ? Ni(r, n) : r.depth > n.depth ? Ht(r, n) : Or(r, n), Se = n);
    }
    function Ho(n) {
      if (n !== null && typeof n != "function") {
        var r = String(n);
        Pi.has(r) || (Pi.add(r), console.error(
          "Expected the last optional `callback` argument to be a function. Instead received: %s.",
          n
        ));
      }
    }
    function Jr(n, r) {
      n = (n = n.constructor) && qn(n) || "ReactClass";
      var u = n + "." + r;
      Xe[u] || (console.error(
        `Can only update a mounting component. This usually means you called %s() outside componentWillMount() on the server. This is a no-op.

Please check the code for the %s component.`,
        r,
        n
      ), Xe[u] = !0);
    }
    function je(n, r, u) {
      var h = n.id;
      n = n.overflow;
      var y = 32 - dl(h) - 1;
      h &= ~(1 << y), u += 1;
      var p = 32 - dl(r) + y;
      if (30 < p) {
        var P = y - y % 5;
        return p = (h & (1 << P) - 1).toString(32), h >>= P, y -= P, {
          id: 1 << 32 - dl(r) + y | u << y | h,
          overflow: p + n
        };
      }
      return {
        id: 1 << p | u << y | h,
        overflow: n
      };
    }
    function Vu(n) {
      return n >>>= 0, n === 0 ? 32 : 31 - (cr(n) / _l | 0) | 0;
    }
    function Ye() {
    }
    function Uo(n, r, u) {
      switch (u = n[u], u === void 0 ? n.push(r) : u !== r && (r.then(Ye, Ye), r = u), r.status) {
        case "fulfilled":
          return r.value;
        case "rejected":
          throw r.reason;
        default:
          switch (typeof r.status == "string" ? r.then(Ye, Ye) : (n = r, n.status = "pending", n.then(
            function(h) {
              if (r.status === "pending") {
                var y = r;
                y.status = "fulfilled", y.value = h;
              }
            },
            function(h) {
              if (r.status === "pending") {
                var y = r;
                y.status = "rejected", y.reason = h;
              }
            }
          )), r.status) {
            case "fulfilled":
              return r.value;
            case "rejected":
              throw r.reason;
          }
          throw Wa = r, ur;
      }
    }
    function Yo() {
      if (Wa === null)
        throw Error(
          "Expected a suspended thenable. This is a bug in React. Please file an issue."
        );
      var n = Wa;
      return Wa = null, n;
    }
    function Go(n, r) {
      return n === r && (n !== 0 || 1 / n === 1 / r) || n !== n && r !== r;
    }
    function Ul() {
      if (Sr === null)
        throw Error(
          `Invalid hook call. Hooks can only be called inside of the body of a function component. This could happen for one of the following reasons:
1. You might have mismatching versions of React and the renderer (such as React DOM)
2. You might be breaking the Rules of Hooks
3. You might have more than one copy of React in the same app
See https://react.dev/link/invalid-hook-call for tips about how to debug and fix this problem.`
        );
      return $l && console.error(
        "Do not call Hooks inside useEffect(...), useMemo(...), or other built-in Hooks. You can only call Hooks at the top level of your React function. For more information, see https://react.dev/link/rules-of-hooks"
      ), Sr;
    }
    function hn() {
      if (0 < $c)
        throw Error("Rendered more hooks than during the previous render");
      return { memoizedState: null, queue: null, next: null };
    }
    function $e() {
      return Dn === null ? Du === null ? (ko = !1, Du = Dn = hn()) : (ko = !0, Dn = Du) : Dn.next === null ? (ko = !1, Dn = Dn.next = hn()) : (ko = !0, Dn = Dn.next), Dn;
    }
    function Cl() {
      var n = So;
      return So = null, n;
    }
    function Ut() {
      $l = !1, Ha = sr = Co = Sr = null, Dl = !1, Du = null, $c = 0, Dn = aa = null;
    }
    function xa(n) {
      return $l && console.error(
        "Context can only be read while React is rendering. In classes, you can read it in the render method or getDerivedStateFromProps. In function components, you can read it directly in the function body, but not inside Hooks like useReducer() or useMemo()."
      ), n._currentValue2;
    }
    function mc(n, r) {
      return typeof r == "function" ? r(n) : r;
    }
    function er(n, r, u) {
      if (n !== mc && (Ya = "useReducer"), Sr = Ul(), Dn = $e(), ko) {
        if (u = Dn.queue, r = u.dispatch, aa !== null) {
          var h = aa.get(u);
          if (h !== void 0) {
            aa.delete(u), u = Dn.memoizedState;
            do {
              var y = h.action;
              $l = !0, u = n(u, y), $l = !1, h = h.next;
            } while (h !== null);
            return Dn.memoizedState = u, [u, r];
          }
        }
        return [Dn.memoizedState, r];
      }
      return $l = !0, n = n === mc ? typeof r == "function" ? r() : r : u !== void 0 ? u(r) : r, $l = !1, Dn.memoizedState = n, n = Dn.queue = { last: null, dispatch: null }, n = n.dispatch = yu.bind(
        null,
        Sr,
        n
      ), [Dn.memoizedState, n];
    }
    function Qu(n, r) {
      if (Sr = Ul(), Dn = $e(), r = r === void 0 ? null : r, Dn !== null) {
        var u = Dn.memoizedState;
        if (u !== null && r !== null) {
          e: {
            var h = u[1];
            if (h === null)
              console.error(
                "%s received a final argument during this render, but not during the previous render. Even though the final argument is optional, its type cannot change between renders.",
                Ya
              ), h = !1;
            else {
              r.length !== h.length && console.error(
                `The final argument passed to %s changed size between renders. The order and size of this array must remain constant.

Previous: %s
Incoming: %s`,
                Ya,
                "[" + r.join(", ") + "]",
                "[" + h.join(", ") + "]"
              );
              for (var y = 0; y < h.length && y < r.length; y++)
                if (!Ro(r[y], h[y])) {
                  h = !1;
                  break e;
                }
              h = !0;
            }
          }
          if (h) return u[0];
        }
      }
      return $l = !0, n = n(), $l = !1, Dn.memoizedState = [n, r], n;
    }
    function yu(n, r, u) {
      if (25 <= $c)
        throw Error(
          "Too many re-renders. React limits the number of renders to prevent an infinite loop."
        );
      if (n === Sr)
        if (Dl = !0, n = { action: u, next: null }, aa === null && (aa = /* @__PURE__ */ new Map()), u = aa.get(r), u === void 0)
          aa.set(r, n);
        else {
          for (r = u; r.next !== null; ) r = r.next;
          r.next = n;
        }
    }
    function Ac() {
      throw Error(
        "A function wrapped in useEffectEvent can't be called during rendering."
      );
    }
    function In() {
      throw Error("startTransition cannot be called during server rendering.");
    }
    function Ic() {
      throw Error("Cannot update optimistic state while rendering.");
    }
    function to(n, r, u) {
      Ul();
      var h = Ua++, y = sr;
      if (typeof n.$$FORM_ACTION == "function") {
        var p = null, P = Ha;
        y = y.formState;
        var M = n.$$IS_SIGNATURE_EQUAL;
        if (y !== null && typeof M == "function") {
          var K = y[1];
          M.call(n, y[2], y[3]) && (p = u !== void 0 ? "p" + u : "k" + Nl(
            JSON.stringify([
              P,
              null,
              h
            ]),
            0
          ), K === p && (qc = h, r = y[0]));
        }
        var L = n.bind(null, r);
        return n = function(ie) {
          L(ie);
        }, typeof L.$$FORM_ACTION == "function" && (n.$$FORM_ACTION = function(ie) {
          ie = L.$$FORM_ACTION(ie), u !== void 0 && (pe(u, "target"), u += "", ie.action = u);
          var fe = ie.data;
          return fe && (p === null && (p = u !== void 0 ? "p" + u : "k" + Nl(
            JSON.stringify([
              P,
              null,
              h
            ]),
            0
          )), fe.append("$ACTION_KEY", p)), ie;
        }), [r, n, !1];
      }
      var J = n.bind(null, r);
      return [
        r,
        function(ie) {
          J(ie);
        },
        !1
      ];
    }
    function Vr(n) {
      var r = jc;
      return jc += 1, So === null && (So = []), Uo(So, n, r);
    }
    function Ta() {
      throw Error("Cache cannot be refreshed during server rendering.");
    }
    function Ku() {
    }
    function qu() {
      if (tl === 0) {
        oa = console.log, Fi = console.info, eu = console.warn, nu = console.error, tu = console.group, Xa = console.groupCollapsed, cs = console.groupEnd;
        var n = {
          configurable: !0,
          enumerable: !0,
          value: Ku,
          writable: !0
        };
        Object.defineProperties(console, {
          info: n,
          log: n,
          warn: n,
          error: n,
          group: n,
          groupCollapsed: n,
          groupEnd: n
        });
      }
      tl++;
    }
    function ro() {
      if (tl--, tl === 0) {
        var n = { configurable: !0, enumerable: !0, writable: !0 };
        Object.defineProperties(console, {
          log: On({}, n, { value: oa }),
          info: On({}, n, { value: Fi }),
          warn: On({}, n, { value: eu }),
          error: On({}, n, { value: nu }),
          group: On({}, n, { value: tu }),
          groupCollapsed: On({}, n, { value: Xa }),
          groupEnd: On({}, n, { value: cs })
        });
      }
      0 > tl && console.error(
        "disabledDepth fell below zero. This is a bug in React. Please file an issue."
      );
    }
    function lo(n) {
      var r = Error.prepareStackTrace;
      if (Error.prepareStackTrace = void 0, n = n.stack, Error.prepareStackTrace = r, n.startsWith(`Error: react-stack-top-frame
`) && (n = n.slice(29)), r = n.indexOf(`
`), r !== -1 && (n = n.slice(r + 1)), r = n.indexOf("react_stack_bottom_frame"), r !== -1 && (r = n.lastIndexOf(
        `
`,
        r
      )), r !== -1)
        n = n.slice(0, r);
      else return "";
      return n;
    }
    function Yl(n) {
      if (Lu === void 0)
        try {
          throw Error();
        } catch (u) {
          var r = u.stack.trim().match(/\n( *(at )?)/);
          Lu = r && r[1] || "", Po = -1 < u.stack.indexOf(`
    at`) ? " (<anonymous>)" : -1 < u.stack.indexOf("@") ? "@unknown:0:0" : "";
        }
      return `
` + Lu + n + Po;
    }
    function Gl(n, r) {
      if (!n || us) return "";
      var u = ru.get(n);
      if (u !== void 0) return u;
      us = !0, u = Error.prepareStackTrace, Error.prepareStackTrace = void 0;
      var h = null;
      h = Wn.H, Wn.H = null, qu();
      try {
        var y = {
          DetermineComponentFrameRoot: function() {
            try {
              if (r) {
                var fe = function() {
                  throw Error();
                };
                if (Object.defineProperty(fe.prototype, "props", {
                  set: function() {
                    throw Error();
                  }
                }), typeof Reflect == "object" && Reflect.construct) {
                  try {
                    Reflect.construct(fe, []);
                  } catch ($) {
                    var oe = $;
                  }
                  Reflect.construct(n, [], fe);
                } else {
                  try {
                    fe.call();
                  } catch ($) {
                    oe = $;
                  }
                  n.call(fe.prototype);
                }
              } else {
                try {
                  throw Error();
                } catch ($) {
                  oe = $;
                }
                (fe = n()) && typeof fe.catch == "function" && fe.catch(function() {
                });
              }
            } catch ($) {
              if ($ && oe && typeof $.stack == "string")
                return [$.stack, oe.stack];
            }
            return [null, null];
          }
        };
        y.DetermineComponentFrameRoot.displayName = "DetermineComponentFrameRoot";
        var p = Object.getOwnPropertyDescriptor(
          y.DetermineComponentFrameRoot,
          "name"
        );
        p && p.configurable && Object.defineProperty(
          y.DetermineComponentFrameRoot,
          "name",
          { value: "DetermineComponentFrameRoot" }
        );
        var P = y.DetermineComponentFrameRoot(), M = P[0], K = P[1];
        if (M && K) {
          var L = M.split(`
`), J = K.split(`
`);
          for (P = p = 0; p < L.length && !L[p].includes(
            "DetermineComponentFrameRoot"
          ); )
            p++;
          for (; P < J.length && !J[P].includes(
            "DetermineComponentFrameRoot"
          ); )
            P++;
          if (p === L.length || P === J.length)
            for (p = L.length - 1, P = J.length - 1; 1 <= p && 0 <= P && L[p] !== J[P]; )
              P--;
          for (; 1 <= p && 0 <= P; p--, P--)
            if (L[p] !== J[P]) {
              if (p !== 1 || P !== 1)
                do
                  if (p--, P--, 0 > P || L[p] !== J[P]) {
                    var ie = `
` + L[p].replace(
                      " at new ",
                      " at "
                    );
                    return n.displayName && ie.includes("<anonymous>") && (ie = ie.replace("<anonymous>", n.displayName)), typeof n == "function" && ru.set(n, ie), ie;
                  }
                while (1 <= p && 0 <= P);
              break;
            }
        }
      } finally {
        us = !1, Wn.H = h, ro(), Error.prepareStackTrace = u;
      }
      return L = (L = n ? n.displayName || n.name : "") ? Yl(L) : "", typeof n == "function" && ru.set(n, L), L;
    }
    function Mc(n) {
      if (typeof n == "string") return Yl(n);
      if (typeof n == "function")
        return n.prototype && n.prototype.isReactComponent ? Gl(n, !0) : Gl(n, !1);
      if (typeof n == "object" && n !== null) {
        switch (n.$$typeof) {
          case Kr:
            return Gl(n.render, !1);
          case Tn:
            return Gl(n.type, !1);
          case jr:
            var r = n, u = r._payload;
            r = r._init;
            try {
              n = r(u);
            } catch {
              return Yl("Lazy");
            }
            return Mc(n);
        }
        if (typeof n.name == "string") {
          e: {
            if (u = n.name, r = n.env, n = n.debugLocation, n != null) {
              n = lo(n);
              var h = n.lastIndexOf(`
`);
              if (n = h === -1 ? n : n.slice(h + 1), n.indexOf(u) !== -1) {
                u = `
` + n;
                break e;
              }
            }
            u = Yl(
              u + (r ? " [" + r + "]" : "")
            );
          }
          return u;
        }
      }
      switch (n) {
        case qr:
          return Yl("SuspenseList");
        case _r:
          return Yl("Suspense");
      }
      return "";
    }
    function Xo(n, r) {
      return (500 < r.byteSize || !1) && r.contentPreamble === null;
    }
    function Zo(n) {
      if (typeof n == "object" && n !== null && typeof n.environmentName == "string") {
        var r = n.environmentName;
        n = [n].slice(0), typeof n[0] == "string" ? n.splice(
          0,
          1,
          "[%s] " + n[0],
          " " + r + " "
        ) : n.splice(0, 0, "[%s]", " " + r + " "), n.unshift(console), r = Z.apply(console.error, n), r();
      } else console.error(n);
      return null;
    }
    function xt(n, r, u, h, y, p, P, M, K, L, J) {
      var ie = /* @__PURE__ */ new Set();
      this.destination = null, this.flushScheduled = !1, this.resumableState = n, this.renderState = r, this.rootFormatContext = u, this.progressiveChunkSize = h === void 0 ? 12800 : h, this.status = 10, this.fatalError = null, this.pendingRootTasks = this.allPendingTasks = this.nextSegmentId = 0, this.completedPreambleSegments = this.completedRootSegment = null, this.byteSize = 0, this.abortableTasks = ie, this.pingedTasks = [], this.clientRenderedBoundaries = [], this.completedBoundaries = [], this.partialBoundaries = [], this.trackedPostpones = null, this.onError = y === void 0 ? Zo : y, this.onPostpone = L === void 0 ? Ye : L, this.onAllReady = p === void 0 ? Ye : p, this.onShellReady = P === void 0 ? Ye : P, this.onShellError = M === void 0 ? Ye : M, this.onFatalError = K === void 0 ? Ye : K, this.formState = J === void 0 ? null : J, this.didWarnForKey = null;
    }
    function io(n, r, u, h, y, p, P, M, K, L, J, ie) {
      var fe = ss();
      return 1e3 < fe - Ps && (Wn.recentlyCreatedOwnerStacks = 0, Ps = fe), r = new xt(
        r,
        u,
        h,
        y,
        p,
        P,
        M,
        K,
        L,
        J,
        ie
      ), u = Tr(
        r,
        0,
        null,
        h,
        !1,
        !1
      ), u.parentFlushed = !0, n = Xl(
        r,
        null,
        n,
        -1,
        null,
        u,
        null,
        null,
        r.abortableTasks,
        null,
        h,
        null,
        Na,
        null,
        null,
        ce,
        null
      ), wr(n), r.pingedTasks.push(n), r;
    }
    function Jo(n, r) {
      n.pingedTasks.push(r), n.pingedTasks.length === 1 && (n.flushScheduled = n.destination !== null, pu(n));
    }
    function Oc(n, r, u, h, y) {
      return u = {
        status: mi,
        rootSegmentID: -1,
        parentFlushed: !1,
        pendingTasks: 0,
        row: r,
        completedSegments: [],
        byteSize: 0,
        fallbackAbortableTasks: u,
        errorDigest: null,
        contentState: ha(),
        fallbackState: ha(),
        contentPreamble: h,
        fallbackPreamble: y,
        trackedContentKeyPath: null,
        trackedFallbackNode: null,
        errorMessage: null,
        errorStack: null,
        errorComponentStack: null
      }, r !== null && (r.pendingTasks++, h = r.boundaries, h !== null && (n.allPendingTasks++, u.pendingTasks++, h.push(u)), n = r.inheritedHoistables, n !== null && ll(u.contentState, n)), u;
    }
    function Xl(n, r, u, h, y, p, P, M, K, L, J, ie, fe, oe, $, Ne, Cn) {
      n.allPendingTasks++, y === null ? n.pendingRootTasks++ : y.pendingTasks++, oe !== null && oe.pendingTasks++;
      var Re = {
        replay: null,
        node: u,
        childIndex: h,
        ping: function() {
          return Jo(n, Re);
        },
        blockedBoundary: y,
        blockedSegment: p,
        blockedPreamble: P,
        hoistableState: M,
        abortSet: K,
        keyPath: L,
        formatContext: J,
        context: ie,
        treeContext: fe,
        row: oe,
        componentStack: $,
        thenableState: r
      };
      return Re.debugTask = Cn, K.add(Re), Re;
    }
    function Vo(n, r, u, h, y, p, P, M, K, L, J, ie, fe, oe, $, Ne) {
      n.allPendingTasks++, p === null ? n.pendingRootTasks++ : p.pendingTasks++, fe !== null && fe.pendingTasks++, u.pendingTasks++;
      var Cn = {
        replay: u,
        node: h,
        childIndex: y,
        ping: function() {
          return Jo(n, Cn);
        },
        blockedBoundary: p,
        blockedSegment: null,
        blockedPreamble: null,
        hoistableState: P,
        abortSet: M,
        keyPath: K,
        formatContext: L,
        context: J,
        treeContext: ie,
        row: fe,
        componentStack: oe,
        thenableState: r
      };
      return Cn.debugTask = Ne, M.add(Cn), Cn;
    }
    function Tr(n, r, u, h, y, p) {
      return {
        status: mi,
        parentFlushed: !1,
        id: -1,
        index: r,
        chunks: [],
        children: [],
        preambleChildren: [],
        parentFormatContext: h,
        boundary: u,
        lastPushedText: y,
        textEmbedded: p
      };
    }
    function kl() {
      if (hl === null || hl.componentStack === null)
        return "";
      var n = hl.componentStack;
      try {
        var r = "";
        if (typeof n.type == "string")
          r += Yl(n.type);
        else if (typeof n.type == "function") {
          if (!n.owner) {
            var u = r, h = n.type, y = h ? h.displayName || h.name : "", p = y ? Yl(y) : "";
            r = u + p;
          }
        } else
          n.owner || (r += Mc(n.type));
        for (; n; )
          u = null, n.debugStack != null ? u = lo(
            n.debugStack
          ) : (p = n, p.stack != null && (u = typeof p.stack != "string" ? p.stack = lo(
            p.stack
          ) : p.stack)), (n = n.owner) && u && (r += `
` + u);
        var P = r;
      } catch (M) {
        P = `
Error generating stack: ` + M.message + `
` + M.stack;
      }
      return P;
    }
    function xu(n, r) {
      if (r != null)
        for (var u = r.length - 1; 0 <= u; u--) {
          var h = r[u];
          if (typeof h.name == "string" || typeof h.time == "number") break;
          if (h.awaited != null) {
            var y = h.debugStack == null ? h.awaited : h;
            if (y.debugStack !== void 0) {
              n.componentStack = {
                parent: n.componentStack,
                type: h,
                owner: y.owner,
                stack: y.debugStack
              }, n.debugTask = y.debugTask;
              break;
            }
          }
        }
    }
    function Qo(n, r) {
      if (r != null)
        for (var u = 0; u < r.length; u++) {
          var h = r[u];
          typeof h.name == "string" && h.debugStack !== void 0 && (n.componentStack = {
            parent: n.componentStack,
            type: h,
            owner: h.owner,
            stack: h.debugStack
          }, n.debugTask = h.debugTask);
        }
    }
    function wr(n) {
      var r = n.node;
      if (typeof r == "object" && r !== null)
        switch (r.$$typeof) {
          case Ca:
            var u = r.type, h = r._owner, y = r._debugStack;
            Qo(n, r._debugInfo), n.debugTask = r._debugTask, n.componentStack = {
              parent: n.componentStack,
              type: u,
              owner: h,
              stack: y
            };
            break;
          case jr:
            Qo(n, r._debugInfo);
            break;
          default:
            typeof r.then == "function" && Qo(n, r._debugInfo);
        }
    }
    function Ko(n) {
      return n === null ? null : {
        parent: n.parent,
        type: "Suspense Fallback",
        owner: n.owner,
        stack: n.stack
      };
    }
    function Hi(n) {
      var r = {};
      return n && Object.defineProperty(r, "componentStack", {
        configurable: !0,
        enumerable: !0,
        get: function() {
          try {
            var u = "", h = n;
            do
              u += Mc(h.type), h = h.parent;
            while (h);
            var y = u;
          } catch (p) {
            y = `
Error generating stack: ` + p.message + `
` + p.stack;
          }
          return Object.defineProperty(r, "componentStack", {
            value: y
          }), y;
        }
      }), r;
    }
    function ao(n, r, u, h, y) {
      n.errorDigest = r, u instanceof Error ? (r = String(u.message), u = String(u.stack)) : (r = typeof u == "object" && u !== null ? Kn(u) : String(u), u = null), y = y ? `Switched to client rendering because the server rendering aborted due to:

` : `Switched to client rendering because the server rendering errored:

`, n.errorMessage = y + r, n.errorStack = u !== null ? y + u : null, n.errorComponentStack = h.componentStack;
    }
    function pr(n, r, u, h) {
      if (n = n.onError, r = h ? h.run(n.bind(null, r, u)) : n(r, u), r != null && typeof r != "string")
        console.error(
          'onError returned something with a type other than "string". onError should return a string and may return null or undefined but must not return anything else. It received something of type "%s" instead',
          typeof r
        );
      else return r;
    }
    function St(n, r, u, h) {
      u = n.onShellError;
      var y = n.onFatalError;
      h ? (h.run(u.bind(null, r)), h.run(y.bind(null, r))) : (u(r), y(r)), n.destination !== null ? (n.status = Ai, n.destination.destroy(r)) : (n.status = 13, n.fatalError = r);
    }
    function Sn(n, r) {
      Tu(n, r.next, r.hoistables);
    }
    function Tu(n, r, u) {
      for (; r !== null; ) {
        u !== null && (ll(r.hoistables, u), r.inheritedHoistables = u);
        var h = r.boundaries;
        if (h !== null) {
          r.boundaries = null;
          for (var y = 0; y < h.length; y++) {
            var p = h[y];
            u !== null && ll(
              p.contentState,
              u
            ), Fl(n, p, null, null);
          }
        }
        if (r.pendingTasks--, 0 < r.pendingTasks) break;
        u = r.hoistables, r = r.next;
      }
    }
    function fi(n, r) {
      var u = r.boundaries;
      if (u !== null && r.pendingTasks === u.length) {
        for (var h = !0, y = 0; y < u.length; y++) {
          var p = u[y];
          if (p.pendingTasks !== 1 || p.parentFlushed || Xo(n, p)) {
            h = !1;
            break;
          }
        }
        h && Tu(n, r, r.hoistables);
      }
    }
    function Sl(n) {
      var r = {
        pendingTasks: 1,
        boundaries: null,
        hoistables: ha(),
        inheritedHoistables: null,
        together: !1,
        next: null
      };
      return n !== null && 0 < n.pendingTasks && (r.pendingTasks++, r.boundaries = [], n.next = r), r;
    }
    function wu(n, r, u, h, y) {
      var p = r.keyPath, P = r.treeContext, M = r.row, K = r.componentStack, L = r.debugTask;
      Qo(r, r.node.props.children._debugInfo), r.keyPath = u, u = h.length;
      var J = null;
      if (r.replay !== null) {
        var ie = r.replay.slots;
        if (ie !== null && typeof ie == "object")
          for (var fe = 0; fe < u; fe++) {
            var oe = y !== "backwards" && y !== "unstable_legacy-backwards" ? fe : u - 1 - fe, $ = h[oe];
            r.row = J = Sl(
              J
            ), r.treeContext = je(P, u, oe);
            var Ne = ie[oe];
            typeof Ne == "number" ? (hi(n, r, Ne, $, oe), delete ie[oe]) : at(n, r, $, oe), --J.pendingTasks === 0 && Sn(n, J);
          }
        else
          for (ie = 0; ie < u; ie++)
            fe = y !== "backwards" && y !== "unstable_legacy-backwards" ? ie : u - 1 - ie, oe = h[fe], it(n, r, oe), r.row = J = Sl(J), r.treeContext = je(P, u, fe), at(n, r, oe, fe), --J.pendingTasks === 0 && Sn(n, J);
      } else if (y !== "backwards" && y !== "unstable_legacy-backwards")
        for (y = 0; y < u; y++)
          ie = h[y], it(n, r, ie), r.row = J = Sl(J), r.treeContext = je(
            P,
            u,
            y
          ), at(n, r, ie, y), --J.pendingTasks === 0 && Sn(n, J);
      else {
        for (y = r.blockedSegment, ie = y.children.length, fe = y.chunks.length, oe = u - 1; 0 <= oe; oe--) {
          $ = h[oe], r.row = J = Sl(
            J
          ), r.treeContext = je(P, u, oe), Ne = Tr(
            n,
            fe,
            null,
            r.formatContext,
            oe === 0 ? y.lastPushedText : !0,
            !0
          ), y.children.splice(ie, 0, Ne), r.blockedSegment = Ne, it(n, r, $);
          try {
            at(n, r, $, oe), ba(
              Ne.chunks,
              n.renderState,
              Ne.lastPushedText,
              Ne.textEmbedded
            ), Ne.status = fr, --J.pendingTasks === 0 && Sn(n, J);
          } catch (Cn) {
            throw Ne.status = n.status === 12 ? gl : fn, Cn;
          }
        }
        r.blockedSegment = y, y.lastPushedText = !1;
      }
      M !== null && J !== null && 0 < J.pendingTasks && (M.pendingTasks++, J.next = M), r.treeContext = P, r.row = M, r.keyPath = p, r.componentStack = K, r.debugTask = L;
    }
    function di(n, r, u, h, y, p) {
      var P = r.thenableState;
      for (r.thenableState = null, Sr = {}, Co = r, sr = n, Ha = u, $l = !1, Ua = Kc = 0, qc = -1, jc = 0, So = P, n = zu(h, y, p); Dl; )
        Dl = !1, Ua = Kc = 0, qc = -1, jc = 0, $c += 1, Dn = null, n = h(y, p);
      return Ut(), n;
    }
    function Zl(n, r, u, h, y, p, P) {
      var M = !1;
      if (p !== 0 && n.formState !== null) {
        var K = r.blockedSegment;
        if (K !== null) {
          M = !0, K = K.chunks;
          for (var L = 0; L < p; L++)
            L === P ? K.push("<!--F!-->") : K.push("<!--F-->");
        }
      }
      p = r.keyPath, r.keyPath = u, y ? (u = r.treeContext, r.treeContext = je(u, 1, 0), at(n, r, h, -1), r.treeContext = u) : M ? at(n, r, h, -1) : Ie(n, r, h, -1), r.keyPath = p;
    }
    function oo(n, r, u, h, y, p) {
      if (typeof h == "function")
        if (h.prototype && h.prototype.isReactComponent) {
          var P = y;
          if ("ref" in y) {
            P = {};
            for (var M in y)
              M !== "ref" && (P[M] = y[M]);
          }
          var K = h.defaultProps;
          if (K) {
            P === y && (P = On({}, P, y));
            for (var L in K)
              P[L] === void 0 && (P[L] = K[L]);
          }
          var J = P, ie = ce, fe = h.contextType;
          if ("contextType" in h && fe !== null && (fe === void 0 || fe.$$typeof !== Qr) && !kr.has(h)) {
            kr.add(h);
            var oe = fe === void 0 ? " However, it is set to undefined. This can be caused by a typo or by mixing up named and default imports. This can also happen due to a circular dependency, so try moving the createContext() call to a separate file." : typeof fe != "object" ? " However, it is set to a " + typeof fe + "." : fe.$$typeof === Dc ? " Did you accidentally pass the Context.Consumer instead?" : " However, it is set to an object with keys {" + Object.keys(fe).join(", ") + "}.";
            console.error(
              "%s defines an invalid contextType. contextType should point to the Context object returned by React.createContext().%s",
              qn(h) || "Component",
              oe
            );
          }
          typeof fe == "object" && fe !== null && (ie = fe._currentValue2);
          var $ = new h(J, ie);
          if (typeof h.getDerivedStateFromProps == "function" && ($.state === null || $.state === void 0)) {
            var Ne = qn(h) || "Component";
            Fn.has(Ne) || (Fn.add(Ne), console.error(
              "`%s` uses `getDerivedStateFromProps` but its initial state is %s. This is not recommended. Instead, define the initial state by assigning an object to `this.state` in the constructor of `%s`. This ensures that `getDerivedStateFromProps` arguments have a consistent shape.",
              Ne,
              $.state === null ? "null" : "undefined",
              Ne
            ));
          }
          if (typeof h.getDerivedStateFromProps == "function" || typeof $.getSnapshotBeforeUpdate == "function") {
            var Cn = null, Re = null, ln = null;
            if (typeof $.componentWillMount == "function" && $.componentWillMount.__suppressDeprecationWarning !== !0 ? Cn = "componentWillMount" : typeof $.UNSAFE_componentWillMount == "function" && (Cn = "UNSAFE_componentWillMount"), typeof $.componentWillReceiveProps == "function" && $.componentWillReceiveProps.__suppressDeprecationWarning !== !0 ? Re = "componentWillReceiveProps" : typeof $.UNSAFE_componentWillReceiveProps == "function" && (Re = "UNSAFE_componentWillReceiveProps"), typeof $.componentWillUpdate == "function" && $.componentWillUpdate.__suppressDeprecationWarning !== !0 ? ln = "componentWillUpdate" : typeof $.UNSAFE_componentWillUpdate == "function" && (ln = "UNSAFE_componentWillUpdate"), Cn !== null || Re !== null || ln !== null) {
              var Jt = qn(h) || "Component", Vt = typeof h.getDerivedStateFromProps == "function" ? "getDerivedStateFromProps()" : "getSnapshotBeforeUpdate()";
              Dr.has(Jt) || (Dr.add(
                Jt
              ), console.error(
                `Unsafe legacy lifecycles will not be called for components using new component APIs.

%s uses %s but also contains the following legacy lifecycles:%s%s%s

The above lifecycles should be removed. Learn more about this warning here:
https://react.dev/link/unsafe-component-lifecycles`,
                Jt,
                Vt,
                Cn !== null ? `
  ` + Cn : "",
                Re !== null ? `
  ` + Re : "",
                ln !== null ? `
  ` + ln : ""
              ));
            }
          }
          var cn = qn(h) || "Component";
          $.render || (h.prototype && typeof h.prototype.render == "function" ? console.error(
            "No `render` method found on the %s instance: did you accidentally return an object from the constructor?",
            cn
          ) : console.error(
            "No `render` method found on the %s instance: you may have forgotten to define `render`.",
            cn
          )), !$.getInitialState || $.getInitialState.isReactClassApproved || $.state || console.error(
            "getInitialState was defined on %s, a plain JavaScript class. This is only supported for classes created using React.createClass. Did you mean to define a state property instead?",
            cn
          ), $.getDefaultProps && !$.getDefaultProps.isReactClassApproved && console.error(
            "getDefaultProps was defined on %s, a plain JavaScript class. This is only supported for classes created using React.createClass. Use a static property to define defaultProps instead.",
            cn
          ), $.contextType && console.error(
            "contextType was defined as an instance property on %s. Use a static property to define contextType instead.",
            cn
          ), h.childContextTypes && !tt.has(h) && (tt.add(h), console.error(
            "%s uses the legacy childContextTypes API which was removed in React 19. Use React.createContext() instead. (https://react.dev/link/legacy-context)",
            cn
          )), h.contextTypes && !or.has(h) && (or.add(h), console.error(
            "%s uses the legacy contextTypes API which was removed in React 19. Use React.createContext() with static contextType instead. (https://react.dev/link/legacy-context)",
            cn
          )), typeof $.componentShouldUpdate == "function" && console.error(
            "%s has a method called componentShouldUpdate(). Did you mean shouldComponentUpdate()? The name is phrased as a question because the function is expected to return a value.",
            cn
          ), h.prototype && h.prototype.isPureReactComponent && typeof $.shouldComponentUpdate < "u" && console.error(
            "%s has a method called shouldComponentUpdate(). shouldComponentUpdate should not be used when extending React.PureComponent. Please extend React.Component if shouldComponentUpdate is used.",
            qn(h) || "A pure component"
          ), typeof $.componentDidUnmount == "function" && console.error(
            "%s has a method called componentDidUnmount(). But there is no such lifecycle method. Did you mean componentWillUnmount()?",
            cn
          ), typeof $.componentDidReceiveProps == "function" && console.error(
            "%s has a method called componentDidReceiveProps(). But there is no such lifecycle method. If you meant to update the state in response to changing props, use componentWillReceiveProps(). If you meant to fetch data or run side-effects or mutations after React has updated the UI, use componentDidUpdate().",
            cn
          ), typeof $.componentWillRecieveProps == "function" && console.error(
            "%s has a method called componentWillRecieveProps(). Did you mean componentWillReceiveProps()?",
            cn
          ), typeof $.UNSAFE_componentWillRecieveProps == "function" && console.error(
            "%s has a method called UNSAFE_componentWillRecieveProps(). Did you mean UNSAFE_componentWillReceiveProps()?",
            cn
          );
          var mt = $.props !== J;
          $.props !== void 0 && mt && console.error(
            "When calling super() in `%s`, make sure to pass up the same props that your component's constructor was passed.",
            cn
          ), $.defaultProps && console.error(
            "Setting defaultProps as an instance property on %s is not supported and will be ignored. Instead, define defaultProps as a static property on %s.",
            cn,
            cn
          ), typeof $.getSnapshotBeforeUpdate != "function" || typeof $.componentDidUpdate == "function" || Zt.has(h) || (Zt.add(h), console.error(
            "%s: getSnapshotBeforeUpdate() should be used with componentDidUpdate(). This component defines getSnapshotBeforeUpdate() only.",
            qn(h)
          )), typeof $.getDerivedStateFromProps == "function" && console.error(
            "%s: getDerivedStateFromProps() is defined as an instance method and will be ignored. Instead, declare it as a static method.",
            cn
          ), typeof $.getDerivedStateFromError == "function" && console.error(
            "%s: getDerivedStateFromError() is defined as an instance method and will be ignored. Instead, declare it as a static method.",
            cn
          ), typeof h.getSnapshotBeforeUpdate == "function" && console.error(
            "%s: getSnapshotBeforeUpdate() is defined as a static method and will be ignored. Instead, declare it as an instance method.",
            cn
          );
          var ni = $.state;
          ni && (typeof ni != "object" || Ti(ni)) && console.error("%s.state: must be set to an object or null", cn), typeof $.getChildContext == "function" && typeof h.childContextTypes != "object" && console.error(
            "%s.getChildContext(): childContextTypes must be defined in order to use getChildContext().",
            cn
          );
          var Pr = $.state !== void 0 ? $.state : null;
          $.updater = Gn, $.props = J, $.state = Pr;
          var he = { queue: [], replace: !1 };
          $._reactInternals = he;
          var vn = h.contextType;
          if ($.context = typeof vn == "object" && vn !== null ? vn._currentValue2 : ce, $.state === J) {
            var bn = qn(h) || "Component";
            fl.has(
              bn
            ) || (fl.add(
              bn
            ), console.error(
              "%s: It is not recommended to assign props directly to state because updates to props won't be reflected in state. In most cases, it is better to use props directly.",
              bn
            ));
          }
          var wn = h.getDerivedStateFromProps;
          if (typeof wn == "function") {
            var Xn = wn(
              J,
              Pr
            );
            if (Xn === void 0) {
              var Ve = qn(h) || "Component";
              Pe.has(Ve) || (Pe.add(Ve), console.error(
                "%s.getDerivedStateFromProps(): A valid state object (or null) must be returned. You have returned undefined.",
                Ve
              ));
            }
            var Oe = Xn == null ? Pr : On({}, Pr, Xn);
            $.state = Oe;
          }
          if (typeof h.getDerivedStateFromProps != "function" && typeof $.getSnapshotBeforeUpdate != "function" && (typeof $.UNSAFE_componentWillMount == "function" || typeof $.componentWillMount == "function")) {
            var dr = $.state;
            if (typeof $.componentWillMount == "function") {
              if ($.componentWillMount.__suppressDeprecationWarning !== !0) {
                var yn = qn(h) || "Unknown";
                Yn[yn] || (console.warn(
                  `componentWillMount has been renamed, and is not recommended for use. See https://react.dev/link/unsafe-component-lifecycles for details.

* Move code from componentWillMount to componentDidMount (preferred in most cases) or the constructor.

Please update the following components: %s`,
                  yn
                ), Yn[yn] = !0);
              }
              $.componentWillMount();
            }
            if (typeof $.UNSAFE_componentWillMount == "function" && $.UNSAFE_componentWillMount(), dr !== $.state && (console.error(
              "%s.componentWillMount(): Assigning directly to this.state is deprecated (except inside a component's constructor). Use setState instead.",
              qn(h) || "Component"
            ), Gn.enqueueReplaceState(
              $,
              $.state,
              null
            )), he.queue !== null && 0 < he.queue.length) {
              var Qt = he.queue, Ln = he.replace;
              if (he.queue = null, he.replace = !1, Ln && Qt.length === 1)
                $.state = Qt[0];
              else {
                for (var zr = Ln ? Qt[0] : $.state, Ao = !0, Br = Ln ? 1 : 0; Br < Qt.length; Br++) {
                  var ti = Qt[Br], ri = typeof ti == "function" ? ti.call(
                    $,
                    zr,
                    J,
                    void 0
                  ) : ti;
                  ri != null && (Ao ? (Ao = !1, zr = On(
                    {},
                    zr,
                    ri
                  )) : On(zr, ri));
                }
                $.state = zr;
              }
            } else he.queue = null;
          }
          var li = lu($);
          if (n.status === 12) throw null;
          $.props !== J && (Za || console.error(
            "It looks like %s is reassigning its own `this.props` while rendering. This is not supported and can lead to confusing bugs.",
            qn(h) || "a component"
          ), Za = !0);
          var zt = r.keyPath;
          r.keyPath = u, Ie(n, r, li, -1), r.keyPath = zt;
        } else {
          if (h.prototype && typeof h.prototype.render == "function") {
            var e = qn(h) || "Unknown";
            vc[e] || (console.error(
              "The <%s /> component appears to have a render method, but doesn't extend React.Component. This is likely to cause errors. Change %s to extend React.Component instead.",
              e,
              e
            ), vc[e] = !0);
          }
          var t = di(
            n,
            r,
            u,
            h,
            y,
            void 0
          );
          if (n.status === 12) throw null;
          var c = Kc !== 0, d = Ua, b = qc;
          if (h.contextTypes) {
            var w = qn(h) || "Unknown";
            iu[w] || (iu[w] = !0, console.error(
              "%s uses the legacy contextTypes API which was removed in React 19. Use React.createContext() with React.useContext() instead. (https://react.dev/link/legacy-context)",
              w
            ));
          }
          if (h && h.childContextTypes && console.error(
            `childContextTypes cannot be defined on a function component.
  %s.childContextTypes = ...`,
            h.displayName || h.name || "Component"
          ), typeof h.getDerivedStateFromProps == "function") {
            var k = qn(h) || "Unknown";
            mo[k] || (console.error(
              "%s: Function components do not support getDerivedStateFromProps.",
              k
            ), mo[k] = !0);
          }
          if (typeof h.contextType == "object" && h.contextType !== null) {
            var A = qn(h) || "Unknown";
            fs[A] || (console.error(
              "%s: Function components do not support contextType.",
              A
            ), fs[A] = !0);
          }
          Zl(
            n,
            r,
            u,
            t,
            c,
            d,
            b
          );
        }
      else if (typeof h == "string") {
        var X = r.blockedSegment;
        if (X === null) {
          var O = y.children, z = r.formatContext, ee = r.keyPath;
          r.formatContext = kc(z, h, y), r.keyPath = u, at(n, r, O, -1), r.formatContext = z, r.keyPath = ee;
        } else {
          var re = un(
            X.chunks,
            h,
            y,
            n.resumableState,
            n.renderState,
            r.blockedPreamble,
            r.hoistableState,
            r.formatContext,
            X.lastPushedText
          );
          X.lastPushedText = !1;
          var te = r.formatContext, j = r.keyPath;
          if (r.keyPath = u, (r.formatContext = kc(
            te,
            h,
            y
          )).insertionMode === me) {
            var we = Tr(
              n,
              0,
              null,
              r.formatContext,
              !1,
              !1
            );
            X.preambleChildren.push(we), r.blockedSegment = we;
            try {
              we.status = 6, at(n, r, re, -1), ba(
                we.chunks,
                n.renderState,
                we.lastPushedText,
                we.textEmbedded
              ), we.status = fr;
            } finally {
              r.blockedSegment = X;
            }
          } else at(n, r, re, -1);
          r.formatContext = te, r.keyPath = j;
          e: {
            var pn = X.chunks, ye = n.resumableState;
            switch (h) {
              case "title":
              case "style":
              case "script":
              case "area":
              case "base":
              case "br":
              case "col":
              case "embed":
              case "hr":
              case "img":
              case "input":
              case "keygen":
              case "link":
              case "meta":
              case "param":
              case "source":
              case "track":
              case "wbr":
                break e;
              case "body":
                if (te.insertionMode <= ke) {
                  ye.hasBody = !0;
                  break e;
                }
                break;
              case "html":
                if (te.insertionMode === De) {
                  ye.hasHtml = !0;
                  break e;
                }
                break;
              case "head":
                if (te.insertionMode <= ke) break e;
            }
            pn.push(kt(h));
          }
          X.lastPushedText = !1;
        }
      } else {
        switch (h) {
          case Lc:
          case jo:
          case $o:
          case Ql:
            var Ue = r.keyPath;
            r.keyPath = u, Ie(n, r, y.children, -1), r.keyPath = Ue;
            return;
          case Al:
            var jn = r.blockedSegment;
            if (jn === null) {
              if (y.mode !== "hidden") {
                var ft = r.keyPath;
                r.keyPath = u, at(n, r, y.children, -1), r.keyPath = ft;
              }
            } else if (y.mode !== "hidden") {
              n.renderState.generateStaticMarkup || jn.chunks.push("<!--&-->"), jn.lastPushedText = !1;
              var an = r.keyPath;
              r.keyPath = u, at(n, r, y.children, -1), r.keyPath = an, n.renderState.generateStaticMarkup || jn.chunks.push("<!--/&-->"), jn.lastPushedText = !1;
            }
            return;
          case qr:
            e: {
              var be = y.children, dt = y.revealOrder;
              if (dt === "forwards" || dt === "backwards" || dt === "unstable_legacy-backwards") {
                if (Ti(be)) {
                  wu(
                    n,
                    r,
                    u,
                    be,
                    dt
                  );
                  break e;
                }
                var hr = G(be);
                if (hr) {
                  var En = hr.call(be);
                  if (En) {
                    co(
                      r,
                      be,
                      -1,
                      En,
                      hr
                    );
                    var zn = En.next();
                    if (!zn.done) {
                      var Nr = [];
                      do
                        Nr.push(zn.value), zn = En.next();
                      while (!zn.done);
                      wu(
                        n,
                        r,
                        u,
                        be,
                        dt
                      );
                    }
                    break e;
                  }
                }
              }
              if (dt === "together") {
                var Kt = r.keyPath, At = r.row, Bn = r.row = Sl(null);
                Bn.boundaries = [], Bn.together = !0, r.keyPath = u, Ie(n, r, be, -1), --Bn.pendingTasks === 0 && Sn(n, Bn), r.keyPath = Kt, r.row = At, At !== null && 0 < Bn.pendingTasks && (At.pendingTasks++, Bn.next = At);
              } else {
                var pt = r.keyPath;
                r.keyPath = u, Ie(n, r, be, -1), r.keyPath = pt;
              }
            }
            return;
          case Bc:
          case Mn:
            throw Error(
              "ReactDOMServer does not yet support scope components."
            );
          case _r:
            e: if (r.replay !== null) {
              var Wr = r.keyPath, rl = r.formatContext, It = r.row;
              r.keyPath = u, r.formatContext = Wt(
                n.resumableState,
                rl
              ), r.row = null;
              var Zn = y.children;
              try {
                at(n, r, Zn, -1);
              } finally {
                r.keyPath = Wr, r.formatContext = rl, r.row = It;
              }
            } else {
              var gr = r.keyPath, Hr = r.formatContext, vr = r.row, Ii = r.blockedBoundary, Ur = r.blockedPreamble, ht = r.hoistableState, vl = r.blockedSegment, Bt = y.fallback, Bu = y.children, ii = /* @__PURE__ */ new Set(), $n = Oc(
                n,
                r.row,
                ii,
                null,
                null
              );
              n.trackedPostpones !== null && ($n.trackedContentKeyPath = u);
              var bl = Tr(
                n,
                vl.chunks.length,
                $n,
                r.formatContext,
                !1,
                !1
              );
              vl.children.push(bl), vl.lastPushedText = !1;
              var br = Tr(
                n,
                0,
                null,
                r.formatContext,
                !1,
                !1
              );
              if (br.parentFlushed = !0, n.trackedPostpones !== null) {
                var xc = r.componentStack, Io = [
                  u[0],
                  "Suspense Fallback",
                  u[2]
                ], zl = [
                  Io[1],
                  Io[2],
                  [],
                  null
                ];
                n.trackedPostpones.workingMap.set(
                  Io,
                  zl
                ), $n.trackedFallbackNode = zl, r.blockedSegment = bl, r.blockedPreamble = $n.fallbackPreamble, r.keyPath = Io, r.formatContext = Li(
                  n.resumableState,
                  Hr
                ), r.componentStack = Ko(
                  xc
                ), bl.status = 6;
                try {
                  at(n, r, Bt, -1), ba(
                    bl.chunks,
                    n.renderState,
                    bl.lastPushedText,
                    bl.textEmbedded
                  ), bl.status = fr;
                } catch (au) {
                  throw bl.status = n.status === 12 ? gl : fn, au;
                } finally {
                  r.blockedSegment = vl, r.blockedPreamble = Ur, r.keyPath = gr, r.formatContext = Hr;
                }
                var Mo = Xl(
                  n,
                  null,
                  Bu,
                  -1,
                  $n,
                  br,
                  $n.contentPreamble,
                  $n.contentState,
                  r.abortSet,
                  u,
                  Wt(
                    n.resumableState,
                    r.formatContext
                  ),
                  r.context,
                  r.treeContext,
                  null,
                  xc,
                  ce,
                  r.debugTask
                );
                wr(Mo), n.pingedTasks.push(Mo);
              } else {
                r.blockedBoundary = $n, r.blockedPreamble = $n.contentPreamble, r.hoistableState = $n.contentState, r.blockedSegment = br, r.keyPath = u, r.formatContext = Wt(
                  n.resumableState,
                  Hr
                ), r.row = null, br.status = 6;
                try {
                  if (at(n, r, Bu, -1), ba(
                    br.chunks,
                    n.renderState,
                    br.lastPushedText,
                    br.textEmbedded
                  ), br.status = fr, Ra($n, br), $n.pendingTasks === 0 && $n.status === mi) {
                    if ($n.status = fr, !Xo(n, $n)) {
                      vr !== null && --vr.pendingTasks === 0 && Sn(n, vr), n.pendingRootTasks === 0 && r.blockedPreamble && uo(n);
                      break e;
                    }
                  } else
                    vr !== null && vr.together && fi(n, vr);
                } catch (au) {
                  if ($n.status = Lt, n.status === 12) {
                    br.status = gl;
                    var Yr = n.fatalError;
                  } else
                    br.status = fn, Yr = au;
                  var Tc = Hi(r.componentStack), Nu = pr(
                    n,
                    Yr,
                    Tc,
                    r.debugTask
                  );
                  ao(
                    $n,
                    Nu,
                    Yr,
                    Tc,
                    !1
                  ), wa(n, $n);
                } finally {
                  r.blockedBoundary = Ii, r.blockedPreamble = Ur, r.hoistableState = ht, r.blockedSegment = vl, r.keyPath = gr, r.formatContext = Hr, r.row = vr;
                }
                var yl = Xl(
                  n,
                  null,
                  Bt,
                  -1,
                  Ii,
                  bl,
                  $n.fallbackPreamble,
                  $n.fallbackState,
                  ii,
                  [u[0], "Suspense Fallback", u[2]],
                  Li(
                    n.resumableState,
                    r.formatContext
                  ),
                  r.context,
                  r.treeContext,
                  r.row,
                  Ko(
                    r.componentStack
                  ),
                  ce,
                  r.debugTask
                );
                wr(yl), n.pingedTasks.push(yl);
              }
            }
            return;
        }
        if (typeof h == "object" && h !== null)
          switch (h.$$typeof) {
            case Kr:
              if ("ref" in y) {
                var wc = {};
                for (var yr in y)
                  yr !== "ref" && (wc[yr] = y[yr]);
              } else wc = y;
              var Oo = di(
                n,
                r,
                u,
                h.render,
                wc,
                p
              );
              Zl(
                n,
                r,
                u,
                Oo,
                Kc !== 0,
                Ua,
                qc
              );
              return;
            case Tn:
              oo(n, r, u, h.type, y, p);
              return;
            case Qr:
              var Va = y.value, ua = y.children, xl = r.context, Tl = r.keyPath, ai = h._currentValue2;
              h._currentValue2 = Va, h._currentRenderer2 !== void 0 && h._currentRenderer2 !== null && h._currentRenderer2 !== Ge && console.error(
                "Detected multiple renderers concurrently rendering the same context provider. This is currently unsupported."
              ), h._currentRenderer2 = Ge;
              var Gr = Se, qt = {
                parent: Gr,
                depth: Gr === null ? 0 : Gr.depth + 1,
                context: h,
                parentValue: ai,
                value: Va
              };
              Se = qt, r.context = qt, r.keyPath = u, Ie(n, r, ua, -1);
              var sa = Se;
              if (sa === null)
                throw Error(
                  "Tried to pop a Context at the root of the app. This is a bug in React."
                );
              sa.context !== h && console.error(
                "The parent context is not the expected context. This is probably a bug in React."
              ), sa.context._currentValue2 = sa.parentValue, h._currentRenderer2 !== void 0 && h._currentRenderer2 !== null && h._currentRenderer2 !== Ge && console.error(
                "Detected multiple renderers concurrently rendering the same context provider. This is currently unsupported."
              ), h._currentRenderer2 = Ge;
              var oi = Se = sa.parent;
              r.context = oi, r.keyPath = Tl, xl !== r.context && console.error(
                "Popping the context provider did not return back to the original snapshot. This is a bug in React."
              );
              return;
            case Dc:
              var Qa = h._context, Mi = y.children;
              typeof Mi != "function" && console.error(
                "A context consumer was rendered with multiple children, or a child that isn't a function. A context consumer expects a single child that is a function. If you did pass a function, make sure there is no trailing or leading whitespace around it."
              );
              var ds = Mi(Qa._currentValue2), Fr = r.keyPath;
              r.keyPath = u, Ie(n, r, ds, -1), r.keyPath = Fr;
              return;
            case jr:
              var Bl = Ss(h);
              if (n.status === 12) throw null;
              oo(n, r, u, Bl, y, p);
              return;
          }
        var pc = "";
        throw (h === void 0 || typeof h == "object" && h !== null && Object.keys(h).length === 0) && (pc += " You likely forgot to export your component from the file it's defined in, or you might have mixed up default and named imports."), Error(
          "Element type is invalid: expected a string (for built-in components) or a class/function (for composite components) but got: " + ((h == null ? h : typeof h) + "." + pc)
        );
      }
    }
    function hi(n, r, u, h, y) {
      var p = r.replay, P = r.blockedBoundary, M = Tr(
        n,
        0,
        null,
        r.formatContext,
        !1,
        !1
      );
      M.id = u, M.parentFlushed = !0;
      try {
        r.replay = null, r.blockedSegment = M, at(n, r, h, y), M.status = fr, P === null ? n.completedRootSegment = M : (Ra(P, M), P.parentFlushed && n.partialBoundaries.push(P));
      } finally {
        r.replay = p, r.blockedSegment = null;
      }
    }
    function gi(n, r, u, h, y, p, P, M, K, L) {
      p = L.nodes;
      for (var J = 0; J < p.length; J++) {
        var ie = p[J];
        if (y === ie[1]) {
          if (ie.length === 4) {
            if (h !== null && h !== ie[0])
              throw Error(
                "Expected the resume to render <" + ie[0] + "> in this slot but instead it rendered <" + h + ">. The tree doesn't match so React will fallback to client rendering."
              );
            var fe = ie[2];
            h = ie[3], y = r.node, r.replay = { nodes: fe, slots: h, pendingTasks: 1 };
            try {
              if (oo(n, r, u, P, M, K), r.replay.pendingTasks === 1 && 0 < r.replay.nodes.length)
                throw Error(
                  "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                );
              r.replay.pendingTasks--;
            } catch (mt) {
              if (typeof mt == "object" && mt !== null && (mt === ur || typeof mt.then == "function"))
                throw r.node === y ? r.replay = L : p.splice(J, 1), mt;
              r.replay.pendingTasks--, P = Hi(r.componentStack), M = n, n = r.blockedBoundary, u = mt, K = h, h = pr(M, u, P, r.debugTask), yi(
                M,
                n,
                fe,
                K,
                u,
                h,
                P,
                !1
              );
            }
            r.replay = L;
          } else {
            if (P !== _r)
              throw Error(
                "Expected the resume to render <Suspense> in this slot but instead it rendered <" + (qn(P) || "Unknown") + ">. The tree doesn't match so React will fallback to client rendering."
              );
            e: {
              L = void 0, h = ie[5], P = ie[2], K = ie[3], y = ie[4] === null ? [] : ie[4][2], ie = ie[4] === null ? null : ie[4][3];
              var oe = r.keyPath, $ = r.formatContext, Ne = r.row, Cn = r.replay, Re = r.blockedBoundary, ln = r.hoistableState, Jt = M.children, Vt = M.fallback, cn = /* @__PURE__ */ new Set();
              M = Oc(
                n,
                r.row,
                cn,
                null,
                null
              ), M.parentFlushed = !0, M.rootSegmentID = h, r.blockedBoundary = M, r.hoistableState = M.contentState, r.keyPath = u, r.formatContext = Wt(
                n.resumableState,
                $
              ), r.row = null, r.replay = { nodes: P, slots: K, pendingTasks: 1 };
              try {
                if (at(n, r, Jt, -1), r.replay.pendingTasks === 1 && 0 < r.replay.nodes.length)
                  throw Error(
                    "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                  );
                if (r.replay.pendingTasks--, M.pendingTasks === 0 && M.status === mi) {
                  M.status = fr, n.completedBoundaries.push(M);
                  break e;
                }
              } catch (mt) {
                M.status = Lt, fe = Hi(r.componentStack), L = pr(
                  n,
                  mt,
                  fe,
                  r.debugTask
                ), ao(M, L, mt, fe, !1), r.replay.pendingTasks--, n.clientRenderedBoundaries.push(M);
              } finally {
                r.blockedBoundary = Re, r.hoistableState = ln, r.replay = Cn, r.keyPath = oe, r.formatContext = $, r.row = Ne;
              }
              M = Vo(
                n,
                null,
                { nodes: y, slots: ie, pendingTasks: 0 },
                Vt,
                -1,
                Re,
                M.fallbackState,
                cn,
                [u[0], "Suspense Fallback", u[2]],
                Li(
                  n.resumableState,
                  r.formatContext
                ),
                r.context,
                r.treeContext,
                r.row,
                Ko(
                  r.componentStack
                ),
                ce,
                r.debugTask
              ), wr(M), n.pingedTasks.push(M);
            }
          }
          p.splice(J, 1);
          break;
        }
      }
    }
    function co(n, r, u, h, y) {
      h === r ? (u !== -1 || n.componentStack === null || typeof n.componentStack.type != "function" || Object.prototype.toString.call(n.componentStack.type) !== "[object GeneratorFunction]" || Object.prototype.toString.call(h) !== "[object Generator]") && (bc || console.error(
        "Using Iterators as children is unsupported and will likely yield unexpected results because enumerating a generator mutates it. You may convert it to an array with `Array.from()` or the `[...spread]` operator before rendering. You can also use an Iterable that can iterate multiple times over the same items."
      ), bc = !0) : r.entries !== y || Ll || (console.error(
        "Using Maps as children is not supported. Use an array of keyed ReactElements instead."
      ), Ll = !0);
    }
    function Ie(n, r, u, h) {
      r.replay !== null && typeof r.replay.slots == "number" ? hi(n, r, r.replay.slots, u, h) : (r.node = u, r.childIndex = h, u = r.componentStack, h = r.debugTask, wr(r), Pl(n, r), r.componentStack = u, r.debugTask = h);
    }
    function Pl(n, r) {
      var u = r.node, h = r.childIndex;
      if (u !== null) {
        if (typeof u == "object") {
          switch (u.$$typeof) {
            case Ca:
              var y = u.type, p = u.key;
              u = u.props;
              var P = u.ref;
              P = P !== void 0 ? P : null;
              var M = r.debugTask, K = qn(y);
              p = p ?? (h === -1 ? 0 : h);
              var L = [r.keyPath, K, p];
              r.replay !== null ? M ? M.run(
                gi.bind(
                  null,
                  n,
                  r,
                  L,
                  K,
                  p,
                  h,
                  y,
                  u,
                  P,
                  r.replay
                )
              ) : gi(
                n,
                r,
                L,
                K,
                p,
                h,
                y,
                u,
                P,
                r.replay
              ) : M ? M.run(
                oo.bind(
                  null,
                  n,
                  r,
                  L,
                  y,
                  u,
                  P
                )
              ) : oo(n, r, L, y, u, P);
              return;
            case ot:
              throw Error(
                "Portals are not currently supported by the server renderer. Render them conditionally so that they only appear on the client render."
              );
            case jr:
              if (y = Ss(u), n.status === 12) throw null;
              Ie(n, r, y, h);
              return;
          }
          if (Ti(u)) {
            Jl(n, r, u, h);
            return;
          }
          if ((p = G(u)) && (y = p.call(u))) {
            if (co(r, u, h, y, p), u = y.next(), !u.done) {
              p = [];
              do
                p.push(u.value), u = y.next();
              while (!u.done);
              Jl(n, r, p, h);
            }
            return;
          }
          if (typeof u.then == "function")
            return r.thenableState = null, Ie(
              n,
              r,
              Vr(u),
              h
            );
          if (u.$$typeof === Qr)
            return Ie(
              n,
              r,
              u._currentValue2,
              h
            );
          throw n = Object.prototype.toString.call(u), Error(
            "Objects are not valid as a React child (found: " + (n === "[object Object]" ? "object with keys {" + Object.keys(u).join(", ") + "}" : n) + "). If you meant to render a collection of children, use an array instead."
          );
        }
        typeof u == "string" ? (r = r.blockedSegment, r !== null && (r.lastPushedText = Ju(
          r.chunks,
          u,
          n.renderState,
          r.lastPushedText
        ))) : typeof u == "number" || typeof u == "bigint" ? (r = r.blockedSegment, r !== null && (r.lastPushedText = Ju(
          r.chunks,
          "" + u,
          n.renderState,
          r.lastPushedText
        ))) : (typeof u == "function" && (n = u.displayName || u.name || "Component", console.error(
          "Functions are not valid as a React child. This may happen if you return %s instead of <%s /> from render. Or maybe you meant to call this function rather than return it.",
          n,
          n
        )), typeof u == "symbol" && console.error(
          `Symbols are not valid as a React child.
  %s`,
          String(u)
        ));
      }
    }
    function it(n, r, u) {
      if (u !== null && typeof u == "object" && (u.$$typeof === Ca || u.$$typeof === ot) && u._store && (!u._store.validated && u.key == null || u._store.validated === 2)) {
        if (typeof u._store != "object")
          throw Error(
            "React Component in warnForMissingKey should have a _store. This error is likely caused by a bug in React. Please file an issue."
          );
        u._store.validated = 1;
        var h = n.didWarnForKey;
        if (h == null && (h = n.didWarnForKey = /* @__PURE__ */ new WeakSet()), n = r.componentStack, n !== null && !h.has(n)) {
          h.add(n);
          var y = qn(u.type);
          h = u._owner;
          var p = n.owner;
          if (n = "", p && typeof p.type < "u") {
            var P = qn(p.type);
            P && (n = `

Check the render method of \`` + P + "`.");
          }
          n || y && (n = `

Check the top-level render call using <` + y + ">."), y = "", h != null && p !== h && (p = null, typeof h.type < "u" ? p = qn(h.type) : typeof h.name == "string" && (p = h.name), p && (y = " It was passed a child from " + p + ".")), h = r.componentStack, r.componentStack = {
            parent: r.componentStack,
            type: u.type,
            owner: u._owner,
            stack: u._debugStack
          }, console.error(
            'Each child in a list should have a unique "key" prop.%s%s See https://react.dev/link/warning-keys for more information.',
            n,
            y
          ), r.componentStack = h;
        }
      }
    }
    function Jl(n, r, u, h) {
      var y = r.keyPath, p = r.componentStack, P = r.debugTask;
      if (Qo(r, r.node._debugInfo), h !== -1 && (r.keyPath = [r.keyPath, "Fragment", h], r.replay !== null)) {
        for (var M = r.replay, K = M.nodes, L = 0; L < K.length; L++) {
          var J = K[L];
          if (J[1] === h) {
            h = J[2], J = J[3], r.replay = { nodes: h, slots: J, pendingTasks: 1 };
            try {
              if (Jl(n, r, u, -1), r.replay.pendingTasks === 1 && 0 < r.replay.nodes.length)
                throw Error(
                  "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                );
              r.replay.pendingTasks--;
            } catch ($) {
              if (typeof $ == "object" && $ !== null && ($ === ur || typeof $.then == "function"))
                throw $;
              r.replay.pendingTasks--;
              var ie = Hi(r.componentStack);
              u = r.blockedBoundary;
              var fe = $, oe = J;
              J = pr(
                n,
                fe,
                ie,
                r.debugTask
              ), yi(
                n,
                u,
                h,
                oe,
                fe,
                J,
                ie,
                !1
              );
            }
            r.replay = M, K.splice(L, 1);
            break;
          }
        }
        r.keyPath = y, r.componentStack = p, r.debugTask = P;
        return;
      }
      if (M = r.treeContext, K = u.length, r.replay !== null && (L = r.replay.slots, L !== null && typeof L == "object")) {
        for (h = 0; h < K; h++)
          J = u[h], r.treeContext = je(
            M,
            K,
            h
          ), fe = L[h], typeof fe == "number" ? (hi(n, r, fe, J, h), delete L[h]) : at(n, r, J, h);
        r.treeContext = M, r.keyPath = y, r.componentStack = p, r.debugTask = P;
        return;
      }
      for (L = 0; L < K; L++)
        h = u[L], it(n, r, h), r.treeContext = je(M, K, L), at(n, r, h, L);
      r.treeContext = M, r.keyPath = y, r.componentStack = p, r.debugTask = P;
    }
    function Tt(n, r, u) {
      if (u.status = Lr, u.rootSegmentID = n.nextSegmentId++, n = u.trackedContentKeyPath, n === null)
        throw Error(
          "It should not be possible to postpone at the root. This is a bug in React."
        );
      var h = u.trackedFallbackNode, y = [], p = r.workingMap.get(n);
      return p === void 0 ? (u = [
        n[1],
        n[2],
        y,
        null,
        h,
        u.rootSegmentID
      ], r.workingMap.set(n, u), ml(u, n[0], r), u) : (p[4] = h, p[5] = u.rootSegmentID, p);
    }
    function Ui(n, r, u, h) {
      h.status = Lr;
      var y = u.keyPath, p = u.blockedBoundary;
      if (p === null)
        h.id = n.nextSegmentId++, r.rootSlots = h.id, n.completedRootSegment !== null && (n.completedRootSegment.status = Lr);
      else {
        if (p !== null && p.status === mi) {
          var P = Tt(
            n,
            r,
            p
          );
          if (p.trackedContentKeyPath === y && u.childIndex === -1) {
            h.id === -1 && (h.id = h.parentFlushed ? p.rootSegmentID : n.nextSegmentId++), P[3] = h.id;
            return;
          }
        }
        if (h.id === -1 && (h.id = h.parentFlushed && p !== null ? p.rootSegmentID : n.nextSegmentId++), u.childIndex === -1)
          y === null ? r.rootSlots = h.id : (u = r.workingMap.get(y), u === void 0 ? (u = [y[1], y[2], [], h.id], ml(u, y[0], r)) : u[3] = h.id);
        else {
          if (y === null) {
            if (n = r.rootSlots, n === null)
              n = r.rootSlots = {};
            else if (typeof n == "number")
              throw Error(
                "It should not be possible to postpone both at the root of an element as well as a slot below. This is a bug in React."
              );
          } else if (p = r.workingMap, P = p.get(y), P === void 0)
            n = {}, P = [y[1], y[2], [], n], p.set(y, P), ml(P, y[0], r);
          else if (n = P[3], n === null)
            n = P[3] = {};
          else if (typeof n == "number")
            throw Error(
              "It should not be possible to postpone both at the root of an element as well as a slot below. This is a bug in React."
            );
          n[u.childIndex] = h.id;
        }
      }
    }
    function wa(n, r) {
      n = n.trackedPostpones, n !== null && (r = r.trackedContentKeyPath, r !== null && (r = n.workingMap.get(r), r !== void 0 && (r.length = 4, r[2] = [], r[3] = null)));
    }
    function Vl(n, r, u) {
      return Vo(
        n,
        u,
        r.replay,
        r.node,
        r.childIndex,
        r.blockedBoundary,
        r.hoistableState,
        r.abortSet,
        r.keyPath,
        r.formatContext,
        r.context,
        r.treeContext,
        r.row,
        r.componentStack,
        ce,
        r.debugTask
      );
    }
    function vi(n, r, u) {
      var h = r.blockedSegment, y = Tr(
        n,
        h.chunks.length,
        null,
        r.formatContext,
        h.lastPushedText,
        !0
      );
      return h.children.push(y), h.lastPushedText = !1, Xl(
        n,
        u,
        r.node,
        r.childIndex,
        r.blockedBoundary,
        y,
        r.blockedPreamble,
        r.hoistableState,
        r.abortSet,
        r.keyPath,
        r.formatContext,
        r.context,
        r.treeContext,
        r.row,
        r.componentStack,
        ce,
        r.debugTask
      );
    }
    function at(n, r, u, h) {
      var y = r.formatContext, p = r.context, P = r.keyPath, M = r.treeContext, K = r.componentStack, L = r.debugTask, J = r.blockedSegment;
      if (J === null) {
        J = r.replay;
        try {
          return Ie(n, r, u, h);
        } catch (oe) {
          if (Ut(), u = oe === ur ? Yo() : oe, n.status !== 12 && typeof u == "object" && u !== null) {
            if (typeof u.then == "function") {
              h = oe === ur ? Cl() : null, n = Vl(
                n,
                r,
                h
              ).ping, u.then(n, n), r.formatContext = y, r.context = p, r.keyPath = P, r.treeContext = M, r.componentStack = K, r.replay = J, r.debugTask = L, xn(p);
              return;
            }
            if (u.message === "Maximum call stack size exceeded") {
              u = oe === ur ? Cl() : null, u = Vl(n, r, u), n.pingedTasks.push(u), r.formatContext = y, r.context = p, r.keyPath = P, r.treeContext = M, r.componentStack = K, r.replay = J, r.debugTask = L, xn(p);
              return;
            }
          }
        }
      } else {
        var ie = J.children.length, fe = J.chunks.length;
        try {
          return Ie(n, r, u, h);
        } catch (oe) {
          if (Ut(), J.children.length = ie, J.chunks.length = fe, u = oe === ur ? Yo() : oe, n.status !== 12 && typeof u == "object" && u !== null) {
            if (typeof u.then == "function") {
              J = u, u = oe === ur ? Cl() : null, n = vi(n, r, u).ping, J.then(n, n), r.formatContext = y, r.context = p, r.keyPath = P, r.treeContext = M, r.componentStack = K, r.debugTask = L, xn(p);
              return;
            }
            if (u.message === "Maximum call stack size exceeded") {
              J = oe === ur ? Cl() : null, J = vi(n, r, J), n.pingedTasks.push(J), r.formatContext = y, r.context = p, r.keyPath = P, r.treeContext = M, r.componentStack = K, r.debugTask = L, xn(p);
              return;
            }
          }
        }
      }
      throw r.formatContext = y, r.context = p, r.keyPath = P, r.treeContext = M, xn(p), u;
    }
    function bi(n) {
      var r = n.blockedBoundary, u = n.blockedSegment;
      u !== null && (u.status = gl, Fl(this, r, n.row, u));
    }
    function yi(n, r, u, h, y, p, P, M) {
      for (var K = 0; K < u.length; K++) {
        var L = u[K];
        if (L.length === 4)
          yi(
            n,
            r,
            L[2],
            L[3],
            y,
            p,
            P,
            M
          );
        else {
          var J = n;
          L = L[5];
          var ie = y, fe = p, oe = P, $ = M, Ne = Oc(
            J,
            null,
            /* @__PURE__ */ new Set(),
            null,
            null
          );
          Ne.parentFlushed = !0, Ne.rootSegmentID = L, Ne.status = Lt, ao(
            Ne,
            fe,
            ie,
            oe,
            $
          ), Ne.parentFlushed && J.clientRenderedBoundaries.push(Ne);
        }
      }
      if (u.length = 0, h !== null) {
        if (r === null)
          throw Error(
            "We should not have any resumable nodes in the shell. This is a bug in React."
          );
        if (r.status !== Lt && (r.status = Lt, ao(
          r,
          p,
          y,
          P,
          M
        ), r.parentFlushed && n.clientRenderedBoundaries.push(r)), typeof h == "object")
          for (var Cn in h) delete h[Cn];
      }
    }
    function pa(n, r, u) {
      var h = n.blockedBoundary, y = n.blockedSegment;
      if (y !== null) {
        if (y.status === 6) return;
        y.status = gl;
      }
      var p = Hi(n.componentStack), P = n.node;
      if (P !== null && typeof P == "object" && xu(n, P._debugInfo), h === null) {
        if (r.status !== 13 && r.status !== Ai) {
          if (h = n.replay, h === null) {
            r.trackedPostpones !== null && y !== null ? (h = r.trackedPostpones, pr(r, u, p, n.debugTask), Ui(r, h, n, y), Fl(r, null, n.row, y)) : (pr(r, u, p, n.debugTask), St(r, u, p, n.debugTask));
            return;
          }
          h.pendingTasks--, h.pendingTasks === 0 && 0 < h.nodes.length && (y = pr(r, u, p, null), yi(
            r,
            null,
            h.nodes,
            h.slots,
            u,
            y,
            p,
            !0
          )), r.pendingRootTasks--, r.pendingRootTasks === 0 && Ea(r);
        }
      } else {
        if (P = r.trackedPostpones, h.status !== Lt) {
          if (P !== null && y !== null)
            return pr(r, u, p, n.debugTask), Ui(r, P, n, y), h.fallbackAbortableTasks.forEach(function(M) {
              return pa(M, r, u);
            }), h.fallbackAbortableTasks.clear(), Fl(r, h, n.row, y);
          h.status = Lt, y = pr(
            r,
            u,
            p,
            n.debugTask
          ), h.status = Lt, ao(h, y, u, p, !0), wa(r, h), h.parentFlushed && r.clientRenderedBoundaries.push(h);
        }
        h.pendingTasks--, p = h.row, p !== null && --p.pendingTasks === 0 && Sn(r, p), h.fallbackAbortableTasks.forEach(function(M) {
          return pa(M, r, u);
        }), h.fallbackAbortableTasks.clear();
      }
      n = n.row, n !== null && --n.pendingTasks === 0 && Sn(r, n), r.allPendingTasks--, r.allPendingTasks === 0 && Er(r);
    }
    function Yi(n, r) {
      try {
        var u = n.renderState, h = u.onHeaders;
        if (h) {
          var y = u.headers;
          if (y) {
            u.headers = null;
            var p = y.preconnects;
            if (y.fontPreloads && (p && (p += ", "), p += y.fontPreloads), y.highImagePreloads && (p && (p += ", "), p += y.highImagePreloads), !r) {
              var P = u.styles.values(), M = P.next();
              e: for (; 0 < y.remainingCapacity && !M.done; M = P.next())
                for (var K = M.value.sheets.values(), L = K.next(); 0 < y.remainingCapacity && !L.done; L = K.next()) {
                  var J = L.value, ie = J.props, fe = ie.href, oe = J.props, $ = Wo(
                    oe.href,
                    "style",
                    {
                      crossOrigin: oe.crossOrigin,
                      integrity: oe.integrity,
                      nonce: oe.nonce,
                      type: oe.type,
                      fetchPriority: oe.fetchPriority,
                      referrerPolicy: oe.referrerPolicy,
                      media: oe.media
                    }
                  );
                  if (0 <= (y.remainingCapacity -= $.length + 2))
                    u.resets.style[fe] = W, p && (p += ", "), p += $, u.resets.style[fe] = typeof ie.crossOrigin == "string" || typeof ie.integrity == "string" ? [ie.crossOrigin, ie.integrity] : W;
                  else break e;
                }
            }
            h(p ? { Link: p } : {});
          }
        }
      } catch (Ne) {
        pr(n, Ne, {}, null);
      }
    }
    function Ea(n) {
      n.trackedPostpones === null && Yi(n, !0), n.trackedPostpones === null && uo(n), n.onShellError = Ye, n = n.onShellReady, n();
    }
    function Er(n) {
      Yi(
        n,
        n.trackedPostpones === null ? !0 : n.completedRootSegment === null || n.completedRootSegment.status !== Lr
      ), uo(n), n = n.onAllReady, n();
    }
    function Ra(n, r) {
      if (r.chunks.length === 0 && r.children.length === 1 && r.children[0].boundary === null && r.children[0].id === -1) {
        var u = r.children[0];
        u.id = r.id, u.parentFlushed = !0, u.status !== fr && u.status !== gl && u.status !== fn || Ra(n, u);
      } else n.completedSegments.push(r);
    }
    function Fl(n, r, u, h) {
      if (u !== null && (--u.pendingTasks === 0 ? Sn(n, u) : u.together && fi(n, u)), n.allPendingTasks--, r === null) {
        if (h !== null && h.parentFlushed) {
          if (n.completedRootSegment !== null)
            throw Error(
              "There can only be one root segment. This is a bug in React."
            );
          n.completedRootSegment = h;
        }
        n.pendingRootTasks--, n.pendingRootTasks === 0 && Ea(n);
      } else if (r.pendingTasks--, r.status !== Lt)
        if (r.pendingTasks === 0) {
          if (r.status === mi && (r.status = fr), h !== null && h.parentFlushed && (h.status === fr || h.status === gl) && Ra(r, h), r.parentFlushed && n.completedBoundaries.push(r), r.status === fr)
            u = r.row, u !== null && ll(u.hoistables, r.contentState), Xo(n, r) || (r.fallbackAbortableTasks.forEach(
              bi,
              n
            ), r.fallbackAbortableTasks.clear(), u !== null && --u.pendingTasks === 0 && Sn(n, u)), n.pendingRootTasks === 0 && n.trackedPostpones === null && r.contentPreamble !== null && uo(n);
          else if (r.status === Lr && (r = r.row, r !== null)) {
            if (n.trackedPostpones !== null) {
              u = n.trackedPostpones;
              var y = r.next;
              if (y !== null && (h = y.boundaries, h !== null))
                for (y.boundaries = null, y = 0; y < h.length; y++) {
                  var p = h[y];
                  Tt(n, u, p), Fl(n, p, null, null);
                }
            }
            --r.pendingTasks === 0 && Sn(n, r);
          }
        } else
          h === null || !h.parentFlushed || h.status !== fr && h.status !== gl || (Ra(r, h), r.completedSegments.length === 1 && r.parentFlushed && n.partialBoundaries.push(r)), r = r.row, r !== null && r.together && fi(n, r);
      n.allPendingTasks === 0 && Er(n);
    }
    function pu(n) {
      if (n.status !== Ai && n.status !== 13) {
        var r = Se, u = Wn.H;
        Wn.H = Ga;
        var h = Wn.A;
        Wn.A = os;
        var y = st;
        st = n;
        var p = Wn.getCurrentStack;
        Wn.getCurrentStack = kl;
        var P = as;
        as = n.resumableState;
        try {
          var M = n.pingedTasks, K;
          for (K = 0; K < M.length; K++) {
            var L = n, J = M[K], ie = J.blockedSegment;
            if (ie === null) {
              var fe = void 0, oe = L;
              if (L = J, L.replay.pendingTasks !== 0) {
                xn(L.context), fe = hl, hl = L;
                try {
                  if (typeof L.replay.slots == "number" ? hi(
                    oe,
                    L,
                    L.replay.slots,
                    L.node,
                    L.childIndex
                  ) : Pl(oe, L), L.replay.pendingTasks === 1 && 0 < L.replay.nodes.length)
                    throw Error(
                      "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                    );
                  L.replay.pendingTasks--, L.abortSet.delete(L), Fl(
                    oe,
                    L.blockedBoundary,
                    L.row,
                    null
                  );
                } catch (Ln) {
                  Ut();
                  var $ = Ln === ur ? Yo() : Ln;
                  if (typeof $ == "object" && $ !== null && typeof $.then == "function") {
                    var Ne = L.ping;
                    $.then(Ne, Ne), L.thenableState = Ln === ur ? Cl() : null;
                  } else {
                    L.replay.pendingTasks--, L.abortSet.delete(L);
                    var Cn = Hi(L.componentStack), Re = void 0, ln = oe, Jt = L.blockedBoundary, Vt = oe.status === 12 ? oe.fatalError : $, cn = Cn, mt = L.replay.nodes, ni = L.replay.slots;
                    Re = pr(
                      ln,
                      Vt,
                      cn,
                      L.debugTask
                    ), yi(
                      ln,
                      Jt,
                      mt,
                      ni,
                      Vt,
                      Re,
                      cn,
                      !1
                    ), oe.pendingRootTasks--, oe.pendingRootTasks === 0 && Ea(oe), oe.allPendingTasks--, oe.allPendingTasks === 0 && Er(oe);
                  }
                } finally {
                  hl = fe;
                }
              }
            } else if (oe = fe = void 0, Re = J, ln = ie, ln.status === mi) {
              ln.status = 6, xn(Re.context), oe = hl, hl = Re;
              var Pr = ln.children.length, he = ln.chunks.length;
              try {
                Pl(L, Re), ba(
                  ln.chunks,
                  L.renderState,
                  ln.lastPushedText,
                  ln.textEmbedded
                ), Re.abortSet.delete(Re), ln.status = fr, Fl(
                  L,
                  Re.blockedBoundary,
                  Re.row,
                  ln
                );
              } catch (Ln) {
                Ut(), ln.children.length = Pr, ln.chunks.length = he;
                var vn = Ln === ur ? Yo() : L.status === 12 ? L.fatalError : Ln;
                if (L.status === 12 && L.trackedPostpones !== null) {
                  var bn = L.trackedPostpones, wn = Hi(Re.componentStack);
                  Re.abortSet.delete(Re), pr(
                    L,
                    vn,
                    wn,
                    Re.debugTask
                  ), Ui(
                    L,
                    bn,
                    Re,
                    ln
                  ), Fl(
                    L,
                    Re.blockedBoundary,
                    Re.row,
                    ln
                  );
                } else if (typeof vn == "object" && vn !== null && typeof vn.then == "function") {
                  ln.status = mi, Re.thenableState = Ln === ur ? Cl() : null;
                  var Xn = Re.ping;
                  vn.then(Xn, Xn);
                } else {
                  var Ve = Hi(
                    Re.componentStack
                  );
                  Re.abortSet.delete(Re), ln.status = fn;
                  var Oe = Re.blockedBoundary, dr = Re.row, yn = Re.debugTask;
                  if (dr !== null && --dr.pendingTasks === 0 && Sn(L, dr), L.allPendingTasks--, fe = pr(
                    L,
                    vn,
                    Ve,
                    yn
                  ), Oe === null)
                    St(
                      L,
                      vn,
                      Ve,
                      yn
                    );
                  else if (Oe.pendingTasks--, Oe.status !== Lt) {
                    Oe.status = Lt, ao(
                      Oe,
                      fe,
                      vn,
                      Ve,
                      !1
                    ), wa(L, Oe);
                    var Qt = Oe.row;
                    Qt !== null && --Qt.pendingTasks === 0 && Sn(L, Qt), Oe.parentFlushed && L.clientRenderedBoundaries.push(Oe), L.pendingRootTasks === 0 && L.trackedPostpones === null && Oe.contentPreamble !== null && uo(L);
                  }
                  L.allPendingTasks === 0 && Er(L);
                }
              } finally {
                hl = oe;
              }
            }
          }
          M.splice(0, K), n.destination !== null && qo(
            n,
            n.destination
          );
        } catch (Ln) {
          M = {}, pr(n, Ln, M, null), St(n, Ln, M, null);
        } finally {
          as = P, Wn.H = u, Wn.A = h, Wn.getCurrentStack = p, u === Ga && xn(r), st = y;
        }
      }
    }
    function Eu(n, r, u) {
      r.preambleChildren.length && u.push(r.preambleChildren);
      for (var h = !1, y = 0; y < r.children.length; y++)
        h = Ru(
          n,
          r.children[y],
          u
        ) || h;
      return h;
    }
    function Ru(n, r, u) {
      var h = r.boundary;
      if (h === null)
        return Eu(
          n,
          r,
          u
        );
      var y = h.contentPreamble, p = h.fallbackPreamble;
      if (y === null || p === null) return !1;
      switch (h.status) {
        case fr:
          if (da(n.renderState, y), n.byteSize += h.byteSize, r = h.completedSegments[0], !r)
            throw Error(
              "A previously unvisited boundary must have exactly one root segment. This is a bug in React."
            );
          return Eu(
            n,
            r,
            u
          );
        case Lr:
          if (n.trackedPostpones !== null) return !0;
        case Lt:
          if (r.status === fr)
            return da(n.renderState, p), Eu(
              n,
              r,
              u
            );
        default:
          return !0;
      }
    }
    function uo(n) {
      if (n.completedRootSegment && n.completedPreambleSegments === null) {
        var r = [], u = n.byteSize, h = Ru(
          n,
          n.completedRootSegment,
          r
        ), y = n.renderState.preamble;
        h === !1 || y.headChunks && y.bodyChunks ? n.completedPreambleSegments = r : n.byteSize = u;
      }
    }
    function so(n, r, u, h) {
      switch (u.parentFlushed = !0, u.status) {
        case mi:
          u.id = n.nextSegmentId++;
        case Lr:
          return h = u.id, u.lastPushedText = !1, u.textEmbedded = !1, n = n.renderState, r.push(Ci), r.push(n.placeholderPrefix), n = h.toString(16), r.push(n), r.push(ac);
        case fr:
          u.status = ca;
          var y = !0, p = u.chunks, P = 0;
          u = u.children;
          for (var M = 0; M < u.length; M++) {
            for (y = u[M]; P < y.index; P++)
              r.push(p[P]);
            y = fo(n, r, y, h);
          }
          for (; P < p.length - 1; P++)
            r.push(p[P]);
          return P < p.length && (y = r.push(p[P])), y;
        case gl:
          return !0;
        default:
          throw Error(
            "Aborted, errored or already flushed boundaries should not be flushed again. This is a bug in React."
          );
      }
    }
    function fo(n, r, u, h) {
      var y = u.boundary;
      if (y === null)
        return so(n, r, u, h);
      if (y.parentFlushed = !0, y.status === Lt) {
        var p = y.row;
        if (p !== null && --p.pendingTasks === 0 && Sn(n, p), !n.renderState.generateStaticMarkup) {
          var P = y.errorDigest, M = y.errorMessage;
          p = y.errorStack, y = y.errorComponentStack, r.push(oc), r.push(yo), P && (r.push(nt), P = Ee(P), r.push(P), r.push(
            Ma
          )), M && (r.push(el), M = Ee(M), r.push(M), r.push(
            Ma
          )), p && (r.push(ze), p = Ee(p), r.push(p), r.push(
            Ma
          )), y && (r.push(Si), p = Ee(y), r.push(p), r.push(
            Ma
          )), r.push(xo);
        }
        return so(n, r, u, h), n = n.renderState.generateStaticMarkup ? !0 : r.push($i), n;
      }
      if (y.status !== fr)
        return y.status === mi && (y.rootSegmentID = n.nextSegmentId++), 0 < y.completedSegments.length && n.partialBoundaries.push(y), Ot(
          r,
          n.renderState,
          y.rootSegmentID
        ), h && ll(h, y.fallbackState), so(n, r, u, h), r.push($i);
      if (!yc && Xo(n, y) && Ja + y.byteSize > n.progressiveChunkSize)
        return y.rootSegmentID = n.nextSegmentId++, n.completedBoundaries.push(y), Ot(
          r,
          n.renderState,
          y.rootSegmentID
        ), so(n, r, u, h), r.push($i);
      if (Ja += y.byteSize, h && ll(h, y.contentState), u = y.row, u !== null && Xo(n, y) && --u.pendingTasks === 0 && Sn(n, u), n.renderState.generateStaticMarkup || r.push(ki), u = y.completedSegments, u.length !== 1)
        throw Error(
          "A previously unvisited boundary must have exactly one root segment. This is a bug in React."
        );
      return fo(n, r, u[0], h), n = n.renderState.generateStaticMarkup ? !0 : r.push($i), n;
    }
    function Pt(n, r, u, h) {
      return xr(
        r,
        n.renderState,
        u.parentFormatContext,
        u.id
      ), fo(n, r, u, h), El(r, u.parentFormatContext);
    }
    function ju(n, r, u) {
      Ja = u.byteSize;
      for (var h = u.completedSegments, y = 0; y < h.length; y++)
        Gi(
          n,
          r,
          u,
          h[y]
        );
      h.length = 0, h = u.row, h !== null && Xo(n, u) && --h.pendingTasks === 0 && Sn(n, h), $t(
        r,
        u.contentState,
        n.renderState
      ), h = n.resumableState, n = n.renderState, y = u.rootSegmentID, u = u.contentState;
      var p = n.stylesToHoist;
      return n.stylesToHoist = !1, r.push(n.startInlineScript), r.push(Be), p ? ((h.instructions & T) === o && (h.instructions |= T, r.push(ta)), (h.instructions & g) === o && (h.instructions |= g, r.push(ut)), (h.instructions & R) === o ? (h.instructions |= R, r.push(fc)) : r.push(dc)) : ((h.instructions & g) === o && (h.instructions |= g, r.push(ut)), r.push(Zc)), h = y.toString(16), r.push(n.boundaryPrefix), r.push(h), r.push(Jc), r.push(n.segmentPrefix), r.push(h), p ? (r.push(Ou), vu(r, u)) : r.push(Vc), u = r.push(Qc), zi(r, n) && u;
    }
    function Gi(n, r, u, h) {
      if (h.status === ca) return !0;
      var y = u.contentState, p = h.id;
      if (p === -1) {
        if ((h.id = u.rootSegmentID) === -1)
          throw Error(
            "A root segment ID must have been assigned by now. This is a bug in React."
          );
        return Pt(
          n,
          r,
          h,
          y
        );
      }
      return p === u.rootSegmentID ? Pt(
        n,
        r,
        h,
        y
      ) : (Pt(n, r, h, y), u = n.resumableState, n = n.renderState, r.push(n.startInlineScript), r.push(Be), (u.instructions & f) === o ? (u.instructions |= f, r.push(Au)) : r.push(Xc), r.push(n.segmentPrefix), p = p.toString(16), r.push(p), r.push(Iu), r.push(n.placeholderPrefix), r.push(p), r = r.push(Mu), r);
    }
    function qo(n, r) {
      try {
        if (!(0 < n.pendingRootTasks)) {
          var u, h = n.completedRootSegment;
          if (h !== null) {
            if (h.status === Lr) return;
            var y = n.completedPreambleSegments;
            if (y === null) return;
            Ja = n.byteSize;
            var p = n.resumableState, P = n.renderState, M = P.preamble, K = M.htmlChunks, L = M.headChunks, J;
            if (K) {
              for (J = 0; J < K.length; J++)
                r.push(K[J]);
              if (L)
                for (J = 0; J < L.length; J++)
                  r.push(L[J]);
              else {
                var ie = lt("head");
                r.push(ie), r.push(Be);
              }
            } else if (L)
              for (J = 0; J < L.length; J++)
                r.push(L[J]);
            var fe = P.charsetChunks;
            for (J = 0; J < fe.length; J++)
              r.push(fe[J]);
            fe.length = 0, P.preconnects.forEach(Rn, r), P.preconnects.clear();
            var oe = P.viewportChunks;
            for (J = 0; J < oe.length; J++)
              r.push(oe[J]);
            oe.length = 0, P.fontPreloads.forEach(Rn, r), P.fontPreloads.clear(), P.highImagePreloads.forEach(Rn, r), P.highImagePreloads.clear(), q = P, P.styles.forEach(et, r), q = null;
            var $ = P.importMapChunks;
            for (J = 0; J < $.length; J++)
              r.push($[J]);
            $.length = 0, P.bootstrapScripts.forEach(Rn, r), P.scripts.forEach(Rn, r), P.scripts.clear(), P.bulkPreloads.forEach(Rn, r), P.bulkPreloads.clear(), p.instructions |= S;
            var Ne = P.hoistableChunks;
            for (J = 0; J < Ne.length; J++)
              r.push(Ne[J]);
            for (p = Ne.length = 0; p < y.length; p++) {
              var Cn = y[p];
              for (P = 0; P < Cn.length; P++)
                fo(n, r, Cn[P], null);
            }
            var Re = n.renderState.preamble, ln = Re.headChunks;
            if (Re.htmlChunks || ln) {
              var Jt = kt("head");
              r.push(Jt);
            }
            var Vt = Re.bodyChunks;
            if (Vt)
              for (y = 0; y < Vt.length; y++)
                r.push(Vt[y]);
            fo(n, r, h, null), n.completedRootSegment = null;
            var cn = n.renderState;
            if (n.allPendingTasks !== 0 || n.clientRenderedBoundaries.length !== 0 || n.completedBoundaries.length !== 0 || n.trackedPostpones !== null && (n.trackedPostpones.rootNodes.length !== 0 || n.trackedPostpones.rootSlots !== null)) {
              var mt = n.resumableState;
              if ((mt.instructions & Y) === o) {
                if (mt.instructions |= Y, r.push(cn.startInlineScript), (mt.instructions & S) === o) {
                  mt.instructions |= S;
                  var ni = "_" + mt.idPrefix + "R_";
                  r.push(Eo);
                  var Pr = Ee(ni);
                  r.push(Pr), r.push(Ae);
                }
                r.push(Be), r.push($r), r.push(Q);
              }
            }
            zi(r, cn);
          }
          var he = n.renderState;
          h = 0;
          var vn = he.viewportChunks;
          for (h = 0; h < vn.length; h++)
            r.push(vn[h]);
          vn.length = 0, he.preconnects.forEach(Rn, r), he.preconnects.clear(), he.fontPreloads.forEach(Rn, r), he.fontPreloads.clear(), he.highImagePreloads.forEach(
            Rn,
            r
          ), he.highImagePreloads.clear(), he.styles.forEach(gu, r), he.scripts.forEach(Rn, r), he.scripts.clear(), he.bulkPreloads.forEach(Rn, r), he.bulkPreloads.clear();
          var bn = he.hoistableChunks;
          for (h = 0; h < bn.length; h++)
            r.push(bn[h]);
          bn.length = 0;
          var wn = n.clientRenderedBoundaries;
          for (u = 0; u < wn.length; u++) {
            var Xn = wn[u];
            he = r;
            var Ve = n.resumableState, Oe = n.renderState, dr = Xn.rootSegmentID, yn = Xn.errorDigest, Qt = Xn.errorMessage, Ln = Xn.errorStack, zr = Xn.errorComponentStack;
            he.push(Oe.startInlineScript), he.push(Be), (Ve.instructions & T) === o ? (Ve.instructions |= T, he.push(ul)) : he.push(ts), he.push(Oe.boundaryPrefix);
            var Ao = dr.toString(16);
            if (he.push(Ao), he.push(za), yn || Qt || Ln || zr) {
              he.push(hc);
              var Br = yt(
                yn || ""
              );
              he.push(Br);
            }
            if (Qt || Ln || zr) {
              he.push(hc);
              var ti = yt(
                Qt || ""
              );
              he.push(ti);
            }
            if (Ln || zr) {
              he.push(hc);
              var ri = yt(
                Ln || ""
              );
              he.push(ri);
            }
            if (zr) {
              he.push(hc);
              var li = yt(zr);
              he.push(li);
            }
            var zt = he.push(
              Ml
            );
            if (!zt) {
              n.destination = null, u++, wn.splice(0, u);
              return;
            }
          }
          wn.splice(0, u);
          var e = n.completedBoundaries;
          for (u = 0; u < e.length; u++)
            if (!ju(
              n,
              r,
              e[u]
            )) {
              n.destination = null, u++, e.splice(0, u);
              return;
            }
          e.splice(0, u), yc = !0;
          var t = n.partialBoundaries;
          for (u = 0; u < t.length; u++) {
            e: {
              wn = n, Xn = r;
              var c = t[u];
              Ja = c.byteSize;
              var d = c.completedSegments;
              for (zt = 0; zt < d.length; zt++)
                if (!Gi(
                  wn,
                  Xn,
                  c,
                  d[zt]
                )) {
                  zt++, d.splice(0, zt);
                  var b = !1;
                  break e;
                }
              d.splice(0, zt);
              var w = c.row;
              w !== null && w.together && c.pendingTasks === 1 && (w.pendingTasks === 1 ? Tu(
                wn,
                w,
                w.hoistables
              ) : w.pendingTasks--), b = $t(
                Xn,
                c.contentState,
                wn.renderState
              );
            }
            if (!b) {
              n.destination = null, u++, t.splice(0, u);
              return;
            }
          }
          t.splice(0, u), yc = !1;
          var k = n.completedBoundaries;
          for (u = 0; u < k.length; u++)
            if (!ju(n, r, k[u])) {
              n.destination = null, u++, k.splice(0, u);
              return;
            }
          k.splice(0, u);
        }
      } finally {
        yc = !1, n.allPendingTasks === 0 && n.clientRenderedBoundaries.length === 0 && n.completedBoundaries.length === 0 && (n.flushScheduled = !1, u = n.resumableState, u.hasBody && (t = kt("body"), r.push(t)), u.hasHtml && (u = kt("html"), r.push(u)), n.abortableTasks.size !== 0 && console.error(
          "There was still abortable task at the root when we closed. This is a bug in React."
        ), n.status = Ai, r.push(null), n.destination = null);
      }
    }
    function _c(n) {
      n.flushScheduled = n.destination !== null, pu(n), n.status === 10 && (n.status = 11), n.trackedPostpones === null && Yi(n, n.pendingRootTasks === 0);
    }
    function xi(n) {
      if (n.flushScheduled === !1 && n.pingedTasks.length === 0 && n.destination !== null) {
        n.flushScheduled = !0;
        var r = n.destination;
        r ? qo(n, r) : n.flushScheduled = !1;
      }
    }
    function Xi(n, r) {
      if (n.status === 13)
        n.status = Ai, r.destroy(n.fatalError);
      else if (n.status !== Ai && n.destination === null) {
        n.destination = r;
        try {
          qo(n, r);
        } catch (u) {
          r = {}, pr(n, u, r, null), St(n, u, r, null);
        }
      }
    }
    function ho(n, r) {
      (n.status === 11 || n.status === 10) && (n.status = 12);
      try {
        var u = n.abortableTasks;
        if (0 < u.size) {
          var h = r === void 0 ? Error("The render was aborted by the server without a reason.") : typeof r == "object" && r !== null && typeof r.then == "function" ? Error("The render was aborted by the server with a promise.") : r;
          n.fatalError = h, u.forEach(function(y) {
            var p = hl, P = Wn.getCurrentStack;
            hl = y, Wn.getCurrentStack = kl;
            try {
              pa(y, n, h);
            } finally {
              hl = p, Wn.getCurrentStack = P;
            }
          }), u.clear();
        }
        n.destination !== null && qo(n, n.destination);
      } catch (y) {
        r = {}, pr(n, y, r, null), St(n, y, r, null);
      }
    }
    function ml(n, r, u) {
      if (r === null) u.rootNodes.push(n);
      else {
        var h = u.workingMap, y = h.get(r);
        y === void 0 && (y = [r[1], r[2], [], null], h.set(r, y), ml(y, r[0], u)), y[2].push(n);
      }
    }
    function Zi() {
    }
    function Ji(n, r, u, h) {
      var y = !1, p = null, P = "", M = !1;
      if (r = dn(
        r ? r.identifierPrefix : void 0
      ), n = io(
        n,
        r,
        Hl(r, u),
        bt(De, null, 0, null),
        1 / 0,
        Zi,
        void 0,
        function() {
          M = !0;
        },
        void 0,
        void 0,
        void 0
      ), _c(n), ho(n, h), Xi(n, {
        push: function(K) {
          return K !== null && (P += K), !0;
        },
        destroy: function(K) {
          y = !0, p = K;
        }
      }), y && p !== h) throw p;
      if (!M)
        throw Error(
          "A component suspended while responding to synchronous input. This will cause the UI to be replaced with a loading indicator. To fix, updates that suspend should be wrapped with startTransition."
        );
      return P;
    }
    var Cu = ks, vs = rf, Ca = Symbol.for("react.transitional.element"), ot = Symbol.for("react.portal"), Ql = Symbol.for("react.fragment"), jo = Symbol.for("react.strict_mode"), $o = Symbol.for("react.profiler"), Dc = Symbol.for("react.consumer"), Qr = Symbol.for("react.context"), Kr = Symbol.for("react.forward_ref"), _r = Symbol.for("react.suspense"), qr = Symbol.for("react.suspense_list"), Tn = Symbol.for("react.memo"), jr = Symbol.for("react.lazy"), Mn = Symbol.for("react.scope"), Al = Symbol.for("react.activity"), Lc = Symbol.for("react.legacy_hidden"), zc = Symbol.for("react.memo_cache_sentinel"), Bc = Symbol.for("react.view_transition"), ec = Symbol.iterator, Ti = Array.isArray, nc = /* @__PURE__ */ new WeakMap(), tc = /* @__PURE__ */ new WeakMap(), Rr = Symbol.for("react.client.reference"), On = Object.assign, en = Object.prototype.hasOwnProperty, Vi = RegExp(
      "^[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), ka = {}, Sa = {}, Qi = new Set(
      "animationIterationCount aspectRatio borderImageOutset borderImageSlice borderImageWidth boxFlex boxFlexGroup boxOrdinalGroup columnCount columns flex flexGrow flexPositive flexShrink flexNegative flexOrder gridArea gridRow gridRowEnd gridRowSpan gridRowStart gridColumn gridColumnEnd gridColumnSpan gridColumnStart fontWeight lineClamp lineHeight opacity order orphans scale tabSize widows zIndex zoom fillOpacity floodOpacity stopOpacity strokeDasharray strokeDashoffset strokeMiterlimit strokeOpacity strokeWidth MozAnimationIterationCount MozBoxFlex MozBoxFlexGroup MozLineClamp msAnimationIterationCount msFlex msZoom msFlexGrow msFlexNegative msFlexOrder msFlexPositive msFlexShrink msGridColumn msGridColumnSpan msGridRow msGridRowSpan WebkitAnimationIterationCount WebkitBoxFlex WebKitBoxFlexGroup WebkitBoxOrdinalGroup WebkitColumnCount WebkitColumns WebkitFlex WebkitFlexGrow WebkitFlexPositive WebkitFlexShrink WebkitLineClamp".split(
        " "
      )
    ), ku = /* @__PURE__ */ new Map([
      ["acceptCharset", "accept-charset"],
      ["htmlFor", "for"],
      ["httpEquiv", "http-equiv"],
      ["crossOrigin", "crossorigin"],
      ["accentHeight", "accent-height"],
      ["alignmentBaseline", "alignment-baseline"],
      ["arabicForm", "arabic-form"],
      ["baselineShift", "baseline-shift"],
      ["capHeight", "cap-height"],
      ["clipPath", "clip-path"],
      ["clipRule", "clip-rule"],
      ["colorInterpolation", "color-interpolation"],
      ["colorInterpolationFilters", "color-interpolation-filters"],
      ["colorProfile", "color-profile"],
      ["colorRendering", "color-rendering"],
      ["dominantBaseline", "dominant-baseline"],
      ["enableBackground", "enable-background"],
      ["fillOpacity", "fill-opacity"],
      ["fillRule", "fill-rule"],
      ["floodColor", "flood-color"],
      ["floodOpacity", "flood-opacity"],
      ["fontFamily", "font-family"],
      ["fontSize", "font-size"],
      ["fontSizeAdjust", "font-size-adjust"],
      ["fontStretch", "font-stretch"],
      ["fontStyle", "font-style"],
      ["fontVariant", "font-variant"],
      ["fontWeight", "font-weight"],
      ["glyphName", "glyph-name"],
      ["glyphOrientationHorizontal", "glyph-orientation-horizontal"],
      ["glyphOrientationVertical", "glyph-orientation-vertical"],
      ["horizAdvX", "horiz-adv-x"],
      ["horizOriginX", "horiz-origin-x"],
      ["imageRendering", "image-rendering"],
      ["letterSpacing", "letter-spacing"],
      ["lightingColor", "lighting-color"],
      ["markerEnd", "marker-end"],
      ["markerMid", "marker-mid"],
      ["markerStart", "marker-start"],
      ["overlinePosition", "overline-position"],
      ["overlineThickness", "overline-thickness"],
      ["paintOrder", "paint-order"],
      ["panose-1", "panose-1"],
      ["pointerEvents", "pointer-events"],
      ["renderingIntent", "rendering-intent"],
      ["shapeRendering", "shape-rendering"],
      ["stopColor", "stop-color"],
      ["stopOpacity", "stop-opacity"],
      ["strikethroughPosition", "strikethrough-position"],
      ["strikethroughThickness", "strikethrough-thickness"],
      ["strokeDasharray", "stroke-dasharray"],
      ["strokeDashoffset", "stroke-dashoffset"],
      ["strokeLinecap", "stroke-linecap"],
      ["strokeLinejoin", "stroke-linejoin"],
      ["strokeMiterlimit", "stroke-miterlimit"],
      ["strokeOpacity", "stroke-opacity"],
      ["strokeWidth", "stroke-width"],
      ["textAnchor", "text-anchor"],
      ["textDecoration", "text-decoration"],
      ["textRendering", "text-rendering"],
      ["transformOrigin", "transform-origin"],
      ["underlinePosition", "underline-position"],
      ["underlineThickness", "underline-thickness"],
      ["unicodeBidi", "unicode-bidi"],
      ["unicodeRange", "unicode-range"],
      ["unitsPerEm", "units-per-em"],
      ["vAlphabetic", "v-alphabetic"],
      ["vHanging", "v-hanging"],
      ["vIdeographic", "v-ideographic"],
      ["vMathematical", "v-mathematical"],
      ["vectorEffect", "vector-effect"],
      ["vertAdvY", "vert-adv-y"],
      ["vertOriginX", "vert-origin-x"],
      ["vertOriginY", "vert-origin-y"],
      ["wordSpacing", "word-spacing"],
      ["writingMode", "writing-mode"],
      ["xmlnsXlink", "xmlns:xlink"],
      ["xHeight", "x-height"]
    ]), _t = {
      button: !0,
      checkbox: !0,
      image: !0,
      hidden: !0,
      radio: !0,
      reset: !0,
      submit: !0
    }, bs = {
      "aria-current": 0,
      "aria-description": 0,
      "aria-details": 0,
      "aria-disabled": 0,
      "aria-hidden": 0,
      "aria-invalid": 0,
      "aria-keyshortcuts": 0,
      "aria-label": 0,
      "aria-roledescription": 0,
      "aria-autocomplete": 0,
      "aria-checked": 0,
      "aria-expanded": 0,
      "aria-haspopup": 0,
      "aria-level": 0,
      "aria-modal": 0,
      "aria-multiline": 0,
      "aria-multiselectable": 0,
      "aria-orientation": 0,
      "aria-placeholder": 0,
      "aria-pressed": 0,
      "aria-readonly": 0,
      "aria-required": 0,
      "aria-selected": 0,
      "aria-sort": 0,
      "aria-valuemax": 0,
      "aria-valuemin": 0,
      "aria-valuenow": 0,
      "aria-valuetext": 0,
      "aria-atomic": 0,
      "aria-busy": 0,
      "aria-live": 0,
      "aria-relevant": 0,
      "aria-dropeffect": 0,
      "aria-grabbed": 0,
      "aria-activedescendant": 0,
      "aria-colcount": 0,
      "aria-colindex": 0,
      "aria-colspan": 0,
      "aria-controls": 0,
      "aria-describedby": 0,
      "aria-errormessage": 0,
      "aria-flowto": 0,
      "aria-labelledby": 0,
      "aria-owns": 0,
      "aria-posinset": 0,
      "aria-rowcount": 0,
      "aria-rowindex": 0,
      "aria-rowspan": 0,
      "aria-setsize": 0,
      "aria-braillelabel": 0,
      "aria-brailleroledescription": 0,
      "aria-colindextext": 0,
      "aria-rowindextext": 0
    }, Il = {}, $u = RegExp(
      "^(aria)-[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), Su = RegExp(
      "^(aria)[A-Z][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), Kl = !1, rc = {
      accept: "accept",
      acceptcharset: "acceptCharset",
      "accept-charset": "acceptCharset",
      accesskey: "accessKey",
      action: "action",
      allowfullscreen: "allowFullScreen",
      alt: "alt",
      as: "as",
      async: "async",
      autocapitalize: "autoCapitalize",
      autocomplete: "autoComplete",
      autocorrect: "autoCorrect",
      autofocus: "autoFocus",
      autoplay: "autoPlay",
      autosave: "autoSave",
      capture: "capture",
      cellpadding: "cellPadding",
      cellspacing: "cellSpacing",
      challenge: "challenge",
      charset: "charSet",
      checked: "checked",
      children: "children",
      cite: "cite",
      class: "className",
      classid: "classID",
      classname: "className",
      cols: "cols",
      colspan: "colSpan",
      content: "content",
      contenteditable: "contentEditable",
      contextmenu: "contextMenu",
      controls: "controls",
      controlslist: "controlsList",
      coords: "coords",
      crossorigin: "crossOrigin",
      dangerouslysetinnerhtml: "dangerouslySetInnerHTML",
      data: "data",
      datetime: "dateTime",
      default: "default",
      defaultchecked: "defaultChecked",
      defaultvalue: "defaultValue",
      defer: "defer",
      dir: "dir",
      disabled: "disabled",
      disablepictureinpicture: "disablePictureInPicture",
      disableremoteplayback: "disableRemotePlayback",
      download: "download",
      draggable: "draggable",
      enctype: "encType",
      enterkeyhint: "enterKeyHint",
      fetchpriority: "fetchPriority",
      for: "htmlFor",
      form: "form",
      formmethod: "formMethod",
      formaction: "formAction",
      formenctype: "formEncType",
      formnovalidate: "formNoValidate",
      formtarget: "formTarget",
      frameborder: "frameBorder",
      headers: "headers",
      height: "height",
      hidden: "hidden",
      high: "high",
      href: "href",
      hreflang: "hrefLang",
      htmlfor: "htmlFor",
      httpequiv: "httpEquiv",
      "http-equiv": "httpEquiv",
      icon: "icon",
      id: "id",
      imagesizes: "imageSizes",
      imagesrcset: "imageSrcSet",
      inert: "inert",
      innerhtml: "innerHTML",
      inputmode: "inputMode",
      integrity: "integrity",
      is: "is",
      itemid: "itemID",
      itemprop: "itemProp",
      itemref: "itemRef",
      itemscope: "itemScope",
      itemtype: "itemType",
      keyparams: "keyParams",
      keytype: "keyType",
      kind: "kind",
      label: "label",
      lang: "lang",
      list: "list",
      loop: "loop",
      low: "low",
      manifest: "manifest",
      marginwidth: "marginWidth",
      marginheight: "marginHeight",
      max: "max",
      maxlength: "maxLength",
      media: "media",
      mediagroup: "mediaGroup",
      method: "method",
      min: "min",
      minlength: "minLength",
      multiple: "multiple",
      muted: "muted",
      name: "name",
      nomodule: "noModule",
      nonce: "nonce",
      novalidate: "noValidate",
      open: "open",
      optimum: "optimum",
      pattern: "pattern",
      placeholder: "placeholder",
      playsinline: "playsInline",
      poster: "poster",
      preload: "preload",
      profile: "profile",
      radiogroup: "radioGroup",
      readonly: "readOnly",
      referrerpolicy: "referrerPolicy",
      rel: "rel",
      required: "required",
      reversed: "reversed",
      role: "role",
      rows: "rows",
      rowspan: "rowSpan",
      sandbox: "sandbox",
      scope: "scope",
      scoped: "scoped",
      scrolling: "scrolling",
      seamless: "seamless",
      selected: "selected",
      shape: "shape",
      size: "size",
      sizes: "sizes",
      span: "span",
      spellcheck: "spellCheck",
      src: "src",
      srcdoc: "srcDoc",
      srclang: "srcLang",
      srcset: "srcSet",
      start: "start",
      step: "step",
      style: "style",
      summary: "summary",
      tabindex: "tabIndex",
      target: "target",
      title: "title",
      type: "type",
      usemap: "useMap",
      value: "value",
      width: "width",
      wmode: "wmode",
      wrap: "wrap",
      about: "about",
      accentheight: "accentHeight",
      "accent-height": "accentHeight",
      accumulate: "accumulate",
      additive: "additive",
      alignmentbaseline: "alignmentBaseline",
      "alignment-baseline": "alignmentBaseline",
      allowreorder: "allowReorder",
      alphabetic: "alphabetic",
      amplitude: "amplitude",
      arabicform: "arabicForm",
      "arabic-form": "arabicForm",
      ascent: "ascent",
      attributename: "attributeName",
      attributetype: "attributeType",
      autoreverse: "autoReverse",
      azimuth: "azimuth",
      basefrequency: "baseFrequency",
      baselineshift: "baselineShift",
      "baseline-shift": "baselineShift",
      baseprofile: "baseProfile",
      bbox: "bbox",
      begin: "begin",
      bias: "bias",
      by: "by",
      calcmode: "calcMode",
      capheight: "capHeight",
      "cap-height": "capHeight",
      clip: "clip",
      clippath: "clipPath",
      "clip-path": "clipPath",
      clippathunits: "clipPathUnits",
      cliprule: "clipRule",
      "clip-rule": "clipRule",
      color: "color",
      colorinterpolation: "colorInterpolation",
      "color-interpolation": "colorInterpolation",
      colorinterpolationfilters: "colorInterpolationFilters",
      "color-interpolation-filters": "colorInterpolationFilters",
      colorprofile: "colorProfile",
      "color-profile": "colorProfile",
      colorrendering: "colorRendering",
      "color-rendering": "colorRendering",
      contentscripttype: "contentScriptType",
      contentstyletype: "contentStyleType",
      cursor: "cursor",
      cx: "cx",
      cy: "cy",
      d: "d",
      datatype: "datatype",
      decelerate: "decelerate",
      descent: "descent",
      diffuseconstant: "diffuseConstant",
      direction: "direction",
      display: "display",
      divisor: "divisor",
      dominantbaseline: "dominantBaseline",
      "dominant-baseline": "dominantBaseline",
      dur: "dur",
      dx: "dx",
      dy: "dy",
      edgemode: "edgeMode",
      elevation: "elevation",
      enablebackground: "enableBackground",
      "enable-background": "enableBackground",
      end: "end",
      exponent: "exponent",
      externalresourcesrequired: "externalResourcesRequired",
      fill: "fill",
      fillopacity: "fillOpacity",
      "fill-opacity": "fillOpacity",
      fillrule: "fillRule",
      "fill-rule": "fillRule",
      filter: "filter",
      filterres: "filterRes",
      filterunits: "filterUnits",
      floodopacity: "floodOpacity",
      "flood-opacity": "floodOpacity",
      floodcolor: "floodColor",
      "flood-color": "floodColor",
      focusable: "focusable",
      fontfamily: "fontFamily",
      "font-family": "fontFamily",
      fontsize: "fontSize",
      "font-size": "fontSize",
      fontsizeadjust: "fontSizeAdjust",
      "font-size-adjust": "fontSizeAdjust",
      fontstretch: "fontStretch",
      "font-stretch": "fontStretch",
      fontstyle: "fontStyle",
      "font-style": "fontStyle",
      fontvariant: "fontVariant",
      "font-variant": "fontVariant",
      fontweight: "fontWeight",
      "font-weight": "fontWeight",
      format: "format",
      from: "from",
      fx: "fx",
      fy: "fy",
      g1: "g1",
      g2: "g2",
      glyphname: "glyphName",
      "glyph-name": "glyphName",
      glyphorientationhorizontal: "glyphOrientationHorizontal",
      "glyph-orientation-horizontal": "glyphOrientationHorizontal",
      glyphorientationvertical: "glyphOrientationVertical",
      "glyph-orientation-vertical": "glyphOrientationVertical",
      glyphref: "glyphRef",
      gradienttransform: "gradientTransform",
      gradientunits: "gradientUnits",
      hanging: "hanging",
      horizadvx: "horizAdvX",
      "horiz-adv-x": "horizAdvX",
      horizoriginx: "horizOriginX",
      "horiz-origin-x": "horizOriginX",
      ideographic: "ideographic",
      imagerendering: "imageRendering",
      "image-rendering": "imageRendering",
      in2: "in2",
      in: "in",
      inlist: "inlist",
      intercept: "intercept",
      k1: "k1",
      k2: "k2",
      k3: "k3",
      k4: "k4",
      k: "k",
      kernelmatrix: "kernelMatrix",
      kernelunitlength: "kernelUnitLength",
      kerning: "kerning",
      keypoints: "keyPoints",
      keysplines: "keySplines",
      keytimes: "keyTimes",
      lengthadjust: "lengthAdjust",
      letterspacing: "letterSpacing",
      "letter-spacing": "letterSpacing",
      lightingcolor: "lightingColor",
      "lighting-color": "lightingColor",
      limitingconeangle: "limitingConeAngle",
      local: "local",
      markerend: "markerEnd",
      "marker-end": "markerEnd",
      markerheight: "markerHeight",
      markermid: "markerMid",
      "marker-mid": "markerMid",
      markerstart: "markerStart",
      "marker-start": "markerStart",
      markerunits: "markerUnits",
      markerwidth: "markerWidth",
      mask: "mask",
      maskcontentunits: "maskContentUnits",
      maskunits: "maskUnits",
      mathematical: "mathematical",
      mode: "mode",
      numoctaves: "numOctaves",
      offset: "offset",
      opacity: "opacity",
      operator: "operator",
      order: "order",
      orient: "orient",
      orientation: "orientation",
      origin: "origin",
      overflow: "overflow",
      overlineposition: "overlinePosition",
      "overline-position": "overlinePosition",
      overlinethickness: "overlineThickness",
      "overline-thickness": "overlineThickness",
      paintorder: "paintOrder",
      "paint-order": "paintOrder",
      panose1: "panose1",
      "panose-1": "panose1",
      pathlength: "pathLength",
      patterncontentunits: "patternContentUnits",
      patterntransform: "patternTransform",
      patternunits: "patternUnits",
      pointerevents: "pointerEvents",
      "pointer-events": "pointerEvents",
      points: "points",
      pointsatx: "pointsAtX",
      pointsaty: "pointsAtY",
      pointsatz: "pointsAtZ",
      popover: "popover",
      popovertarget: "popoverTarget",
      popovertargetaction: "popoverTargetAction",
      prefix: "prefix",
      preservealpha: "preserveAlpha",
      preserveaspectratio: "preserveAspectRatio",
      primitiveunits: "primitiveUnits",
      property: "property",
      r: "r",
      radius: "radius",
      refx: "refX",
      refy: "refY",
      renderingintent: "renderingIntent",
      "rendering-intent": "renderingIntent",
      repeatcount: "repeatCount",
      repeatdur: "repeatDur",
      requiredextensions: "requiredExtensions",
      requiredfeatures: "requiredFeatures",
      resource: "resource",
      restart: "restart",
      result: "result",
      results: "results",
      rotate: "rotate",
      rx: "rx",
      ry: "ry",
      scale: "scale",
      security: "security",
      seed: "seed",
      shaperendering: "shapeRendering",
      "shape-rendering": "shapeRendering",
      slope: "slope",
      spacing: "spacing",
      specularconstant: "specularConstant",
      specularexponent: "specularExponent",
      speed: "speed",
      spreadmethod: "spreadMethod",
      startoffset: "startOffset",
      stddeviation: "stdDeviation",
      stemh: "stemh",
      stemv: "stemv",
      stitchtiles: "stitchTiles",
      stopcolor: "stopColor",
      "stop-color": "stopColor",
      stopopacity: "stopOpacity",
      "stop-opacity": "stopOpacity",
      strikethroughposition: "strikethroughPosition",
      "strikethrough-position": "strikethroughPosition",
      strikethroughthickness: "strikethroughThickness",
      "strikethrough-thickness": "strikethroughThickness",
      string: "string",
      stroke: "stroke",
      strokedasharray: "strokeDasharray",
      "stroke-dasharray": "strokeDasharray",
      strokedashoffset: "strokeDashoffset",
      "stroke-dashoffset": "strokeDashoffset",
      strokelinecap: "strokeLinecap",
      "stroke-linecap": "strokeLinecap",
      strokelinejoin: "strokeLinejoin",
      "stroke-linejoin": "strokeLinejoin",
      strokemiterlimit: "strokeMiterlimit",
      "stroke-miterlimit": "strokeMiterlimit",
      strokewidth: "strokeWidth",
      "stroke-width": "strokeWidth",
      strokeopacity: "strokeOpacity",
      "stroke-opacity": "strokeOpacity",
      suppresscontenteditablewarning: "suppressContentEditableWarning",
      suppresshydrationwarning: "suppressHydrationWarning",
      surfacescale: "surfaceScale",
      systemlanguage: "systemLanguage",
      tablevalues: "tableValues",
      targetx: "targetX",
      targety: "targetY",
      textanchor: "textAnchor",
      "text-anchor": "textAnchor",
      textdecoration: "textDecoration",
      "text-decoration": "textDecoration",
      textlength: "textLength",
      textrendering: "textRendering",
      "text-rendering": "textRendering",
      to: "to",
      transform: "transform",
      transformorigin: "transformOrigin",
      "transform-origin": "transformOrigin",
      typeof: "typeof",
      u1: "u1",
      u2: "u2",
      underlineposition: "underlinePosition",
      "underline-position": "underlinePosition",
      underlinethickness: "underlineThickness",
      "underline-thickness": "underlineThickness",
      unicode: "unicode",
      unicodebidi: "unicodeBidi",
      "unicode-bidi": "unicodeBidi",
      unicoderange: "unicodeRange",
      "unicode-range": "unicodeRange",
      unitsperem: "unitsPerEm",
      "units-per-em": "unitsPerEm",
      unselectable: "unselectable",
      valphabetic: "vAlphabetic",
      "v-alphabetic": "vAlphabetic",
      values: "values",
      vectoreffect: "vectorEffect",
      "vector-effect": "vectorEffect",
      version: "version",
      vertadvy: "vertAdvY",
      "vert-adv-y": "vertAdvY",
      vertoriginx: "vertOriginX",
      "vert-origin-x": "vertOriginX",
      vertoriginy: "vertOriginY",
      "vert-origin-y": "vertOriginY",
      vhanging: "vHanging",
      "v-hanging": "vHanging",
      videographic: "vIdeographic",
      "v-ideographic": "vIdeographic",
      viewbox: "viewBox",
      viewtarget: "viewTarget",
      visibility: "visibility",
      vmathematical: "vMathematical",
      "v-mathematical": "vMathematical",
      vocab: "vocab",
      widths: "widths",
      wordspacing: "wordSpacing",
      "word-spacing": "wordSpacing",
      writingmode: "writingMode",
      "writing-mode": "writingMode",
      x1: "x1",
      x2: "x2",
      x: "x",
      xchannelselector: "xChannelSelector",
      xheight: "xHeight",
      "x-height": "xHeight",
      xlinkactuate: "xlinkActuate",
      "xlink:actuate": "xlinkActuate",
      xlinkarcrole: "xlinkArcrole",
      "xlink:arcrole": "xlinkArcrole",
      xlinkhref: "xlinkHref",
      "xlink:href": "xlinkHref",
      xlinkrole: "xlinkRole",
      "xlink:role": "xlinkRole",
      xlinkshow: "xlinkShow",
      "xlink:show": "xlinkShow",
      xlinktitle: "xlinkTitle",
      "xlink:title": "xlinkTitle",
      xlinktype: "xlinkType",
      "xlink:type": "xlinkType",
      xmlbase: "xmlBase",
      "xml:base": "xmlBase",
      xmllang: "xmlLang",
      "xml:lang": "xmlLang",
      xmlns: "xmlns",
      "xml:space": "xmlSpace",
      xmlnsxlink: "xmlnsXlink",
      "xmlns:xlink": "xmlnsXlink",
      xmlspace: "xmlSpace",
      y1: "y1",
      y2: "y2",
      y: "y",
      ychannelselector: "yChannelSelector",
      z: "z",
      zoomandpan: "zoomAndPan"
    }, Yt = {}, wi = /^on./, es = /^on[^A-Z]/, Pa = RegExp(
      "^(aria)-[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), Nc = RegExp(
      "^(aria)[A-Z][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), Fa = /^(?:webkit|moz|o)[A-Z]/, Gt = /^-ms-/, Dt = /-(.)/g, Ki = /;\s*$/, Pn = {}, gn = {}, ma = !1, go = !1, Wc = /["'&<>]/, vo = /([A-Z])/g, Pu = /^ms-/, ys = /^[\u0000-\u001F ]*j[\r\n\t]*a[\r\n\t]*v[\r\n\t]*a[\r\n\t]*s[\r\n\t]*c[\r\n\t]*r[\r\n\t]*i[\r\n\t]*p[\r\n\t]*t[\r\n\t]*:/i, Wn = Cu.__CLIENT_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, pi = vs.__DOM_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, ns = Object.freeze({
      pending: !1,
      data: null,
      method: null,
      action: null
    }), i = pi.d;
    pi.d = {
      f: i.f,
      r: i.r,
      D: function(n) {
        var r = st || null;
        if (r) {
          var u = r.resumableState, h = r.renderState;
          if (typeof n == "string" && n) {
            if (!u.dnsResources.hasOwnProperty(n)) {
              u.dnsResources[n] = I, u = h.headers;
              var y, p;
              (p = u && 0 < u.remainingCapacity) && (p = (y = "<" + Nn(n) + ">; rel=dns-prefetch", 0 <= (u.remainingCapacity -= y.length + 2))), p ? (h.resets.dns[n] = I, u.preconnects && (u.preconnects += ", "), u.preconnects += y) : (y = [], _e(y, { href: n, rel: "dns-prefetch" }), h.preconnects.add(y));
            }
            xi(r);
          }
        } else i.D(n);
      },
      C: function(n, r) {
        var u = st || null;
        if (u) {
          var h = u.resumableState, y = u.renderState;
          if (typeof n == "string" && n) {
            var p = r === "use-credentials" ? "credentials" : typeof r == "string" ? "anonymous" : "default";
            if (!h.connectResources[p].hasOwnProperty(n)) {
              h.connectResources[p][n] = I, h = y.headers;
              var P, M;
              if (M = h && 0 < h.remainingCapacity) {
                if (M = "<" + Nn(n) + ">; rel=preconnect", typeof r == "string") {
                  var K = Fc(
                    r,
                    "crossOrigin"
                  );
                  M += '; crossorigin="' + K + '"';
                }
                M = (P = M, 0 <= (h.remainingCapacity -= P.length + 2));
              }
              M ? (y.resets.connect[p][n] = I, h.preconnects && (h.preconnects += ", "), h.preconnects += P) : (p = [], _e(p, {
                rel: "preconnect",
                href: n,
                crossOrigin: r
              }), y.preconnects.add(p));
            }
            xi(u);
          }
        } else i.C(n, r);
      },
      L: function(n, r, u) {
        var h = st || null;
        if (h) {
          var y = h.resumableState, p = h.renderState;
          if (r && n) {
            switch (r) {
              case "image":
                if (u)
                  var P = u.imageSrcSet, M = u.imageSizes, K = u.fetchPriority;
                var L = P ? P + `
` + (M || "") : n;
                if (y.imageResources.hasOwnProperty(L)) return;
                y.imageResources[L] = W, y = p.headers;
                var J;
                y && 0 < y.remainingCapacity && typeof P != "string" && K === "high" && (J = Wo(n, r, u), 0 <= (y.remainingCapacity -= J.length + 2)) ? (p.resets.image[L] = W, y.highImagePreloads && (y.highImagePreloads += ", "), y.highImagePreloads += J) : (y = [], _e(
                  y,
                  On(
                    {
                      rel: "preload",
                      href: P ? void 0 : n,
                      as: r
                    },
                    u
                  )
                ), K === "high" ? p.highImagePreloads.add(y) : (p.bulkPreloads.add(y), p.preloads.images.set(L, y)));
                break;
              case "style":
                if (y.styleResources.hasOwnProperty(n)) return;
                P = [], _e(
                  P,
                  On({ rel: "preload", href: n, as: r }, u)
                ), y.styleResources[n] = !u || typeof u.crossOrigin != "string" && typeof u.integrity != "string" ? W : [u.crossOrigin, u.integrity], p.preloads.stylesheets.set(n, P), p.bulkPreloads.add(P);
                break;
              case "script":
                if (y.scriptResources.hasOwnProperty(n)) return;
                P = [], p.preloads.scripts.set(n, P), p.bulkPreloads.add(P), _e(
                  P,
                  On({ rel: "preload", href: n, as: r }, u)
                ), y.scriptResources[n] = !u || typeof u.crossOrigin != "string" && typeof u.integrity != "string" ? W : [u.crossOrigin, u.integrity];
                break;
              default:
                if (y.unknownResources.hasOwnProperty(r)) {
                  if (P = y.unknownResources[r], P.hasOwnProperty(n))
                    return;
                } else
                  P = {}, y.unknownResources[r] = P;
                if (P[n] = W, (y = p.headers) && 0 < y.remainingCapacity && r === "font" && (L = Wo(n, r, u), 0 <= (y.remainingCapacity -= L.length + 2)))
                  p.resets.font[n] = W, y.fontPreloads && (y.fontPreloads += ", "), y.fontPreloads += L;
                else
                  switch (y = [], n = On(
                    { rel: "preload", href: n, as: r },
                    u
                  ), _e(y, n), r) {
                    case "font":
                      p.fontPreloads.add(y);
                      break;
                    default:
                      p.bulkPreloads.add(y);
                  }
            }
            xi(h);
          }
        } else i.L(n, r, u);
      },
      m: function(n, r) {
        var u = st || null;
        if (u) {
          var h = u.resumableState, y = u.renderState;
          if (n) {
            var p = r && typeof r.as == "string" ? r.as : "script";
            switch (p) {
              case "script":
                if (h.moduleScriptResources.hasOwnProperty(n))
                  return;
                p = [], h.moduleScriptResources[n] = !r || typeof r.crossOrigin != "string" && typeof r.integrity != "string" ? W : [r.crossOrigin, r.integrity], y.preloads.moduleScripts.set(n, p);
                break;
              default:
                if (h.moduleUnknownResources.hasOwnProperty(p)) {
                  var P = h.unknownResources[p];
                  if (P.hasOwnProperty(n)) return;
                } else
                  P = {}, h.moduleUnknownResources[p] = P;
                p = [], P[n] = W;
            }
            _e(
              p,
              On({ rel: "modulepreload", href: n }, r)
            ), y.bulkPreloads.add(p), xi(u);
          }
        } else i.m(n, r);
      },
      X: function(n, r) {
        var u = st || null;
        if (u) {
          var h = u.resumableState, y = u.renderState;
          if (n) {
            var p = h.scriptResources.hasOwnProperty(
              n
            ) ? h.scriptResources[n] : void 0;
            p !== I && (h.scriptResources[n] = I, r = On({ src: n, async: !0 }, r), p && (p.length === 2 && ga(r, p), n = y.preloads.scripts.get(n)) && (n.length = 0), n = [], y.scripts.add(n), rt(n, r), xi(u));
          }
        } else i.X(n, r);
      },
      S: function(n, r, u) {
        var h = st || null;
        if (h) {
          var y = h.resumableState, p = h.renderState;
          if (n) {
            r = r || "default";
            var P = p.styles.get(r), M = y.styleResources.hasOwnProperty(n) ? y.styleResources[n] : void 0;
            M !== I && (y.styleResources[n] = I, P || (P = {
              precedence: Ee(r),
              rules: [],
              hrefs: [],
              sheets: /* @__PURE__ */ new Map()
            }, p.styles.set(r, P)), r = {
              state: x,
              props: On(
                {
                  rel: "stylesheet",
                  href: n,
                  "data-precedence": r
                },
                u
              )
            }, M && (M.length === 2 && ga(r.props, M), (p = p.preloads.stylesheets.get(n)) && 0 < p.length ? p.length = 0 : r.state = E), P.sheets.set(n, r), xi(h));
          }
        } else i.S(n, r, u);
      },
      M: function(n, r) {
        var u = st || null;
        if (u) {
          var h = u.resumableState, y = u.renderState;
          if (n) {
            var p = h.moduleScriptResources.hasOwnProperty(n) ? h.moduleScriptResources[n] : void 0;
            p !== I && (h.moduleScriptResources[n] = I, r = On(
              { src: n, type: "module", async: !0 },
              r
            ), p && (p.length === 2 && ga(r, p), n = y.preloads.moduleScripts.get(n)) && (n.length = 0), n = [], y.scripts.add(n), rt(n, r), xi(u));
          }
        } else i.M(n, r);
      }
    };
    var o = 0, f = 1, g = 2, T = 4, R = 8, S = 32, Y = 64, I = null, W = [];
    Object.freeze(W);
    var q = null, Q = "<\/script>", ae = /(<\/|<)(s)(cript)/gi, ve = {}, De = 0, ke = 1, xe = 2, me = 3, Ce = 4, _n = 5, Le = 6, Je = 7, on = 8, Me = 9, Hn = /* @__PURE__ */ new Map(), nn = ' style="', nr = ":", sn = ";", Ke = " ", tn = '="', Ae = '"', rn = '=""', Ft = Ee(
      "javascript:throw new Error('React form unexpectedly submitted.')"
    ), Be = ">", qi = "/>", Ei = !1, tr = !1, il = !1, al = !1, ol = !1, ql = !1, Ri = !1, wt = !1, Aa = !1, Ia = !1, lc = !1, Hc = `addEventListener("submit",function(a){if(!a.defaultPrevented){var c=a.target,d=a.submitter,e=c.action,b=d;if(d){var f=d.getAttribute("formAction");null!=f&&(e=f,b=null)}"javascript:throw new Error('React form unexpectedly submitted.')"===e&&(a.preventDefault(),b?(a=document.createElement("input"),a.name=b.name,a.value=b.value,b.parentNode.insertBefore(a,b),b=new FormData(c),a.parentNode.removeChild(a)):b=new FormData(c),a=c.ownerDocument||c,(a.$$reactFormReplay=a.$$reactFormReplay||[]).push(c,d,b))}});`, bo = /(<\/|<)(s)(tyle)/gi, ji = `
`, rr = /^[a-zA-Z][a-zA-Z:_\.\-\d]*$/, ic = /* @__PURE__ */ new Map(), lr = /* @__PURE__ */ new Map(), $r = "requestAnimationFrame(function(){$RT=performance.now()});", Ci = '<template id="', ac = '"></template>', ki = "<!--$-->", Uc = '<!--$?--><template id="', ct = '"></template>', oc = "<!--$!-->", $i = "<!--/$-->", yo = "<template", Ma = '"', nt = ' data-dgst="', el = ' data-msg="', ze = ' data-stck="', Si = ' data-cstck="', xo = "></template>", ea = '<div hidden id="', Un = '">', Cr = "</div>", na = '<svg aria-hidden="true" style="display:none" id="', Oa = '">', cl = "</svg>", Xt = '<math aria-hidden="true" style="display:none" id="', To = '">', ir = "</math>", Fu = '<table hidden id="', _a = '">', cc = "</table>", Yc = '<table hidden><tbody id="', Gc = '">', jl = "</tbody></table>", Da = '<table hidden><tr id="', La = '">', ar = "</tr></table>", uc = '<table hidden><colgroup id="', sc = '">', mu = "</colgroup></table>", Au = '$RS=function(a,b){a=document.getElementById(a);b=document.getElementById(b);for(a.parentNode.removeChild(a);a.firstChild;)b.parentNode.insertBefore(a.firstChild,b);b.parentNode.removeChild(b)};$RS("', Xc = '$RS("', Iu = '","', Mu = '")<\/script>', ut = `$RB=[];$RV=function(a){$RT=performance.now();for(var b=0;b<a.length;b+=2){var c=a[b],e=a[b+1];null!==e.parentNode&&e.parentNode.removeChild(e);var f=c.parentNode;if(f){var g=c.previousSibling,h=0;do{if(c&&8===c.nodeType){var d=c.data;if("/$"===d||"/&"===d)if(0===h)break;else h--;else"$"!==d&&"$?"!==d&&"$~"!==d&&"$!"!==d&&"&"!==d||h++}d=c.nextSibling;f.removeChild(c);c=d}while(c);for(;e.firstChild;)f.insertBefore(e.firstChild,c);g.data="$";g._reactRetry&&requestAnimationFrame(g._reactRetry)}}a.length=0};
$RC=function(a,b){if(b=document.getElementById(b))(a=document.getElementById(a))?(a.previousSibling.data="$~",$RB.push(a,b),2===$RB.length&&("number"!==typeof $RT?requestAnimationFrame($RV.bind(null,$RB)):(a=performance.now(),setTimeout($RV.bind(null,$RB),2300>a&&2E3<a?2300-a:$RT+300-a)))):b.parentNode.removeChild(b)};`, Zc = '$RC("', fc = `$RM=new Map;$RR=function(n,w,p){function u(q){this._p=null;q()}for(var r=new Map,t=document,h,b,e=t.querySelectorAll("link[data-precedence],style[data-precedence]"),v=[],k=0;b=e[k++];)"not all"===b.getAttribute("media")?v.push(b):("LINK"===b.tagName&&$RM.set(b.getAttribute("href"),b),r.set(b.dataset.precedence,h=b));e=0;b=[];var l,a;for(k=!0;;){if(k){var f=p[e++];if(!f){k=!1;e=0;continue}var c=!1,m=0;var d=f[m++];if(a=$RM.get(d)){var g=a._p;c=!0}else{a=t.createElement("link");a.href=d;a.rel=
"stylesheet";for(a.dataset.precedence=l=f[m++];g=f[m++];)a.setAttribute(g,f[m++]);g=a._p=new Promise(function(q,x){a.onload=u.bind(a,q);a.onerror=u.bind(a,x)});$RM.set(d,a)}d=a.getAttribute("media");!g||d&&!matchMedia(d).matches||b.push(g);if(c)continue}else{a=v[e++];if(!a)break;l=a.getAttribute("data-precedence");a.removeAttribute("media")}c=r.get(l)||h;c===h&&(h=a);r.set(l,a);c?c.parentNode.insertBefore(a,c.nextSibling):(c=t.head,c.insertBefore(a,c.firstChild))}if(p=document.getElementById(n))p.previousSibling.data=
"$~";Promise.all(b).then($RC.bind(null,n,w),$RX.bind(null,n,"CSS failed to load"))};$RR("`, dc = '$RR("', Jc = '","', Ou = '",', Vc = '"', Qc = ")<\/script>", ta = '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};', ul = '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};;$RX("', ts = '$RX("', za = '"', hc = ",", Ml = ")<\/script>", wo = /[<\u2028\u2029]/g, ra = /[&><\u2028\u2029]/g, gc = ' media="not all" data-precedence="', la = '" data-href="', rs = '">', ls = "</style>", Ol = !1, po = !0, nl = [], ia = ' data-precedence="', Ba = '" data-href="', sl = " ", _u = '">', is = "</style>", Eo = ' id="', l = "[", a = ",[", s = ",", v = "]", x = 0, E = 1, C = 2, _ = 3, m = /[<>\r\n]/g, D = /["';,\r\n]/g, U = "", Z = Function.prototype.bind, se = Symbol.for("react.client.reference"), ce = {};
    Object.freeze(ce);
    var Ge = {}, Se = null, Xe = {}, Yn = {}, Fn = /* @__PURE__ */ new Set(), Zt = /* @__PURE__ */ new Set(), Dr = /* @__PURE__ */ new Set(), fl = /* @__PURE__ */ new Set(), Pe = /* @__PURE__ */ new Set(), or = /* @__PURE__ */ new Set(), tt = /* @__PURE__ */ new Set(), kr = /* @__PURE__ */ new Set(), Pi = /* @__PURE__ */ new Set(), Gn = {
      enqueueSetState: function(n, r, u) {
        var h = n._reactInternals;
        h.queue === null ? Jr(n, "setState") : (h.queue.push(r), u != null && Ho(u));
      },
      enqueueReplaceState: function(n, r, u) {
        n = n._reactInternals, n.replace = !0, n.queue = [r], u != null && Ho(u);
      },
      enqueueForceUpdate: function(n, r) {
        n._reactInternals.queue === null ? Jr(n, "forceUpdate") : r != null && Ho(r);
      }
    }, Na = { id: 1, overflow: "" }, dl = Math.clz32 ? Math.clz32 : Vu, cr = Math.log, _l = Math.LN2, ur = Error(
      "Suspense Exception: This is not a real error! It's an implementation detail of `use` to interrupt the current render. You must either rethrow it immediately, or move the `use` call outside of the `try/catch` block. Capturing without rethrowing will lead to unexpected behavior.\n\nTo handle async errors, wrap your component in an error boundary, or call the promise's `.catch` method and pass the result to `use`."
    ), Wa = null, Ro = typeof Object.is == "function" ? Object.is : Go, Sr = null, Co = null, sr = null, Ha = null, Du = null, Dn = null, ko = !1, Dl = !1, Kc = 0, Ua = 0, qc = -1, jc = 0, So = null, aa = null, $c = 0, $l = !1, Ya, Ga = {
      readContext: xa,
      use: function(n) {
        if (n !== null && typeof n == "object") {
          if (typeof n.then == "function")
            return Vr(n);
          if (n.$$typeof === Qr)
            return xa(n);
        }
        throw Error(
          "An unsupported type was passed to use(): " + String(n)
        );
      },
      useContext: function(n) {
        return Ya = "useContext", Ul(), n._currentValue2;
      },
      useMemo: Qu,
      useReducer: er,
      useRef: function(n) {
        Sr = Ul(), Dn = $e();
        var r = Dn.memoizedState;
        return r === null ? (n = { current: n }, Object.seal(n), Dn.memoizedState = n) : r;
      },
      useState: function(n) {
        return Ya = "useState", er(mc, n);
      },
      useInsertionEffect: Ye,
      useLayoutEffect: Ye,
      useCallback: function(n, r) {
        return Qu(function() {
          return n;
        }, r);
      },
      useImperativeHandle: Ye,
      useEffect: Ye,
      useDebugValue: Ye,
      useDeferredValue: function(n, r) {
        return Ul(), r !== void 0 ? r : n;
      },
      useTransition: function() {
        return Ul(), [!1, In];
      },
      useId: function() {
        var n = Co.treeContext, r = n.overflow;
        n = n.id, n = (n & ~(1 << 32 - dl(n) - 1)).toString(32) + r;
        var u = as;
        if (u === null)
          throw Error(
            "Invalid hook call. Hooks can only be called inside of the body of a function component."
          );
        return r = Kc++, n = "_" + u.idPrefix + "R_" + n, 0 < r && (n += "H" + r.toString(32)), n + "_";
      },
      useSyncExternalStore: function(n, r, u) {
        if (u === void 0)
          throw Error(
            "Missing getServerSnapshot, which is required for server-rendered content. Will revert to client rendering."
          );
        return u();
      },
      useOptimistic: function(n) {
        return Ul(), [n, Ic];
      },
      useActionState: to,
      useFormState: to,
      useHostTransitionStatus: function() {
        return Ul(), ns;
      },
      useMemoCache: function(n) {
        for (var r = Array(n), u = 0; u < n; u++)
          r[u] = zc;
        return r;
      },
      useCacheRefresh: function() {
        return Ta;
      },
      useEffectEvent: function() {
        return Ac;
      }
    }, as = null, hl = null, os = {
      getCacheForType: function() {
        throw Error("Not implemented.");
      },
      cacheSignal: function() {
        throw Error("Not implemented.");
      },
      getOwner: function() {
        return hl === null ? null : hl.componentStack;
      }
    }, tl = 0, oa, Fi, eu, nu, tu, Xa, cs;
    Ku.__reactDisabledLog = !0;
    var Lu, Po, us = !1, ru = new (typeof WeakMap == "function" ? WeakMap : Map)(), xs = {
      react_stack_bottom_frame: function(n, r, u) {
        return n(r, u);
      }
    }, zu = xs.react_stack_bottom_frame.bind(xs), Ts = {
      react_stack_bottom_frame: function(n) {
        return n.render();
      }
    }, lu = Ts.react_stack_bottom_frame.bind(Ts), Fo = {
      react_stack_bottom_frame: function(n) {
        var r = n._init;
        return r(n._payload);
      }
    }, Ss = Fo.react_stack_bottom_frame.bind(Fo), Ps = 0;
    if (typeof performance == "object" && typeof performance.now == "function")
      var ei = performance, ss = function() {
        return ei.now();
      };
    else {
      var Fs = Date;
      ss = function() {
        return Fs.now();
      };
    }
    var Lt = 4, mi = 0, fr = 1, ca = 2, gl = 3, fn = 4, Lr = 5, Ai = 14, st = null, vc = {}, iu = {}, fs = {}, mo = {}, Za = !1, bc = !1, Ll = !1, Ja = 0, yc = !1;
    Vs.renderToStaticMarkup = function(n, r) {
      return Ji(
        n,
        r,
        !0,
        'The server used "renderToStaticMarkup" which does not support Suspense. If you intended to have the server wait for the suspended component please switch to "renderToReadableStream" which supports Suspense on the server'
      );
    }, Vs.renderToString = function(n, r) {
      return Ji(
        n,
        r,
        !1,
        'The server used "renderToString" which does not support Suspense. If you intended for this Suspense boundary to render the fallback content on the server consider throwing an Error somewhere within the Suspense boundary. If you intended to have the server wait for the suspended component please switch to "renderToReadableStream" which supports Suspense on the server'
      );
    }, Vs.version = "19.2.4";
  }()), Vs;
}
var Ls = {};
/**
 * @license React
 * react-dom-server.browser.development.js
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
var Ff;
function zf() {
  return Ff || (Ff = 1, process.env.NODE_ENV !== "production" && function() {
    function Te(e, t, c, d) {
      return "" + t + (c === "s" ? "\\73 " : "\\53 ") + d;
    }
    function Vn(e, t, c, d) {
      return "" + t + (c === "s" ? "\\u0073" : "\\u0053") + d;
    }
    function G(e) {
      return e === null || typeof e != "object" ? null : (e = Su && e[Su] || e["@@iterator"], typeof e == "function" ? e : null);
    }
    function vt(e) {
      return e = Object.prototype.toString.call(e), e.slice(8, e.length - 1);
    }
    function Qn(e) {
      var t = JSON.stringify(e);
      return '"' + e + '"' === t ? e : t;
    }
    function kn(e) {
      switch (typeof e) {
        case "string":
          return JSON.stringify(
            10 >= e.length ? e : e.slice(0, 10) + "..."
          );
        case "object":
          return Kl(e) ? "[...]" : e !== null && e.$$typeof === wi ? "client" : (e = vt(e), e === "Object" ? "{...}" : e);
        case "function":
          return e.$$typeof === wi ? "client" : (e = e.displayName || e.name) ? "function " + e : "function";
        default:
          return String(e);
      }
    }
    function Qe(e) {
      if (typeof e == "string") return e;
      switch (e) {
        case Vi:
          return "Suspense";
        case ka:
          return "SuspenseList";
      }
      if (typeof e == "object")
        switch (e.$$typeof) {
          case en:
            return Qe(e.render);
          case Sa:
            return Qe(e.type);
          case Qi:
            var t = e._payload;
            e = e._init;
            try {
              return Qe(e(t));
            } catch {
            }
        }
      return "";
    }
    function Kn(e, t) {
      var c = vt(e);
      if (c !== "Object" && c !== "Array") return c;
      var d = -1, b = 0;
      if (Kl(e))
        if (Yt.has(e)) {
          var w = Yt.get(e);
          c = "<" + Qe(w) + ">";
          for (var k = 0; k < e.length; k++) {
            var A = e[k];
            A = typeof A == "string" ? A : typeof A == "object" && A !== null ? "{" + Kn(A) + "}" : "{" + kn(A) + "}", "" + k === t ? (d = c.length, b = A.length, c += A) : c = 15 > A.length && 40 > c.length + A.length ? c + A : c + "{...}";
          }
          c += "</" + Qe(w) + ">";
        } else {
          for (c = "[", w = 0; w < e.length; w++)
            0 < w && (c += ", "), k = e[w], k = typeof k == "object" && k !== null ? Kn(k) : kn(k), "" + w === t ? (d = c.length, b = k.length, c += k) : c = 10 > k.length && 40 > c.length + k.length ? c + k : c + "...";
          c += "]";
        }
      else if (e.$$typeof === Bc)
        c = "<" + Qe(e.type) + "/>";
      else {
        if (e.$$typeof === wi) return "client";
        if (rc.has(e)) {
          for (c = rc.get(e), c = "<" + (Qe(c) || "..."), w = Object.keys(e), k = 0; k < w.length; k++) {
            c += " ", A = w[k], c += Qn(A) + "=";
            var X = e[A], O = A === t && typeof X == "object" && X !== null ? Kn(X) : kn(X);
            typeof X != "string" && (O = "{" + O + "}"), A === t ? (d = c.length, b = O.length, c += O) : c = 10 > O.length && 40 > c.length + O.length ? c + O : c + "...";
          }
          c += ">";
        } else {
          for (c = "{", w = Object.keys(e), k = 0; k < w.length; k++)
            0 < k && (c += ", "), A = w[k], c += Qn(A) + ": ", X = e[A], X = typeof X == "object" && X !== null ? Kn(X) : kn(X), A === t ? (d = c.length, b = X.length, c += X) : c = 10 > X.length && 40 > c.length + X.length ? c + X : c + "...";
          c += "}";
        }
      }
      return t === void 0 ? c : -1 < d && 0 < b ? (e = " ".repeat(d) + "^".repeat(b), `
  ` + c + `
  ` + e) : `
  ` + c;
    }
    function Nl(e, t) {
      var c = e.length & 3, d = e.length - c, b = t;
      for (t = 0; t < d; ) {
        var w = e.charCodeAt(t) & 255 | (e.charCodeAt(++t) & 255) << 8 | (e.charCodeAt(++t) & 255) << 16 | (e.charCodeAt(++t) & 255) << 24;
        ++t, w = 3432918353 * (w & 65535) + ((3432918353 * (w >>> 16) & 65535) << 16) & 4294967295, w = w << 15 | w >>> 17, w = 461845907 * (w & 65535) + ((461845907 * (w >>> 16) & 65535) << 16) & 4294967295, b ^= w, b = b << 13 | b >>> 19, b = 5 * (b & 65535) + ((5 * (b >>> 16) & 65535) << 16) & 4294967295, b = (b & 65535) + 27492 + (((b >>> 16) + 58964 & 65535) << 16);
      }
      switch (w = 0, c) {
        case 3:
          w ^= (e.charCodeAt(t + 2) & 255) << 16;
        case 2:
          w ^= (e.charCodeAt(t + 1) & 255) << 8;
        case 1:
          w ^= e.charCodeAt(t) & 255, w = 3432918353 * (w & 65535) + ((3432918353 * (w >>> 16) & 65535) << 16) & 4294967295, w = w << 15 | w >>> 17, b ^= 461845907 * (w & 65535) + ((461845907 * (w >>> 16) & 65535) << 16) & 4294967295;
      }
      return b ^= e.length, b ^= b >>> 16, b = 2246822507 * (b & 65535) + ((2246822507 * (b >>> 16) & 65535) << 16) & 4294967295, b ^= b >>> 13, b = 3266489909 * (b & 65535) + ((3266489909 * (b >>> 16) & 65535) << 16) & 4294967295, (b ^ b >>> 16) >>> 0;
    }
    function Et(e) {
      Pa.push(e), es.port2.postMessage(null);
    }
    function jt(e) {
      setTimeout(function() {
        throw e;
      });
    }
    function H(e, t) {
      if (t.byteLength !== 0)
        if (2048 < t.byteLength)
          0 < Dt && (e.enqueue(
            new Uint8Array(Gt.buffer, 0, Dt)
          ), Gt = new Uint8Array(2048), Dt = 0), e.enqueue(t);
        else {
          var c = Gt.length - Dt;
          c < t.byteLength && (c === 0 ? e.enqueue(Gt) : (Gt.set(
            t.subarray(0, c),
            Dt
          ), e.enqueue(Gt), t = t.subarray(c)), Gt = new Uint8Array(2048), Dt = 0), Gt.set(t, Dt), Dt += t.byteLength;
        }
    }
    function pe(e, t) {
      return H(e, t), !0;
    }
    function Ar(e) {
      Gt && 0 < Dt && (e.enqueue(
        new Uint8Array(Gt.buffer, 0, Dt)
      ), Gt = null, Dt = 0);
    }
    function ne(e) {
      return Ki.encode(e);
    }
    function N(e) {
      return e = Ki.encode(e), 2048 < e.byteLength && console.error(
        "precomputed chunks must be smaller than the view size configured for this host. This is a bug in React."
      ), e;
    }
    function Zr(e) {
      return e.byteLength;
    }
    function zo(e, t) {
      typeof e.error == "function" ? e.error(t) : e.close();
    }
    function _i(e) {
      return typeof Symbol == "function" && Symbol.toStringTag && e[Symbol.toStringTag] || e.constructor.name || "Object";
    }
    function Bo(e) {
      try {
        return Di(e), !1;
      } catch {
        return !0;
      }
    }
    function Di(e) {
      return "" + e;
    }
    function Nt(e, t) {
      if (Bo(e))
        return console.error(
          "The provided `%s` attribute is an unsupported type %s. This value must be coerced to a string before using it here.",
          t,
          _i(e)
        ), Di(e);
    }
    function Ee(e, t) {
      if (Bo(e))
        return console.error(
          "The provided `%s` CSS property is an unsupported type %s. This value must be coerced to a string before using it here.",
          t,
          _i(e)
        ), Di(e);
    }
    function Ir(e) {
      if (Bo(e))
        return console.error(
          "The provided HTML markup uses a value of unsupported type %s. This value must be coerced to a string before using it here.",
          _i(e)
        ), Di(e);
    }
    function Rt(e) {
      return gn.call(Wc, e) ? !0 : gn.call(go, e) ? !1 : ma.test(e) ? Wc[e] = !0 : (go[e] = !0, console.error("Invalid attribute name: `%s`", e), !1);
    }
    function dn(e, t) {
      ys[t.type] || t.onChange || t.onInput || t.readOnly || t.disabled || t.value == null || console.error(
        e === "select" ? "You provided a `value` prop to a form field without an `onChange` handler. This will render a read-only field. If the field should be mutable use `defaultValue`. Otherwise, set `onChange`." : "You provided a `value` prop to a form field without an `onChange` handler. This will render a read-only field. If the field should be mutable use `defaultValue`. Otherwise, set either `onChange` or `readOnly`."
      ), t.onChange || t.readOnly || t.disabled || t.checked == null || console.error(
        "You provided a `checked` prop to a form field without an `onChange` handler. This will render a read-only field. If the field should be mutable use `defaultChecked`. Otherwise, set either `onChange` or `readOnly`."
      );
    }
    function bt(e, t) {
      if (gn.call(pi, t) && pi[t])
        return !0;
      if (i.test(t)) {
        if (e = "aria-" + t.slice(4).toLowerCase(), e = Wn.hasOwnProperty(e) ? e : null, e == null)
          return console.error(
            "Invalid ARIA attribute `%s`. ARIA attributes follow the pattern aria-* and must be lowercase.",
            t
          ), pi[t] = !0;
        if (t !== e)
          return console.error(
            "Invalid ARIA attribute `%s`. Did you mean `%s`?",
            t,
            e
          ), pi[t] = !0;
      }
      if (ns.test(t)) {
        if (e = t.toLowerCase(), e = Wn.hasOwnProperty(e) ? e : null, e == null) return pi[t] = !0, !1;
        t !== e && (console.error(
          "Unknown ARIA attribute `%s`. Did you mean `%s`?",
          t,
          e
        ), pi[t] = !0);
      }
      return !0;
    }
    function kc(e, t) {
      var c = [], d;
      for (d in t)
        bt(e, d) || c.push(d);
      t = c.map(function(b) {
        return "`" + b + "`";
      }).join(", "), c.length === 1 ? console.error(
        "Invalid aria prop %s on <%s> tag. For details, see https://react.dev/link/invalid-aria-props",
        t,
        e
      ) : 1 < c.length && console.error(
        "Invalid aria props %s on <%s> tag. For details, see https://react.dev/link/invalid-aria-props",
        t,
        e
      );
    }
    function eo(e, t, c, d) {
      if (gn.call(g, t) && g[t])
        return !0;
      var b = t.toLowerCase();
      if (b === "onfocusin" || b === "onfocusout")
        return console.error(
          "React uses onFocus and onBlur instead of onFocusIn and onFocusOut. All React events are normalized to bubble, so onFocusIn and onFocusOut are not needed/supported by React."
        ), g[t] = !0;
      if (typeof c == "function" && (e === "form" && t === "action" || e === "input" && t === "formAction" || e === "button" && t === "formAction"))
        return !0;
      if (T.test(t))
        return R.test(t) && console.error(
          "Invalid event handler property `%s`. React events use the camelCase naming convention, for example `onClick`.",
          t
        ), g[t] = !0;
      if (S.test(t) || Y.test(t)) return !0;
      if (b === "innerhtml")
        return console.error(
          "Directly setting property `innerHTML` is not permitted. For more information, lookup documentation on `dangerouslySetInnerHTML`."
        ), g[t] = !0;
      if (b === "aria")
        return console.error(
          "The `aria` attribute is reserved for future use in React. Pass individual `aria-` attributes instead."
        ), g[t] = !0;
      if (b === "is" && c !== null && c !== void 0 && typeof c != "string")
        return console.error(
          "Received a `%s` for a string attribute `is`. If this is expected, cast the value to a string.",
          typeof c
        ), g[t] = !0;
      if (typeof c == "number" && isNaN(c))
        return console.error(
          "Received NaN for the `%s` attribute. If this is expected, cast the value to a string.",
          t
        ), g[t] = !0;
      if (f.hasOwnProperty(b)) {
        if (b = f[b], b !== t)
          return console.error(
            "Invalid DOM property `%s`. Did you mean `%s`?",
            t,
            b
          ), g[t] = !0;
      } else if (t !== b)
        return console.error(
          "React does not recognize the `%s` prop on a DOM element. If you intentionally want it to appear in the DOM as a custom attribute, spell it as lowercase `%s` instead. If you accidentally passed it from a parent component, remove it from the DOM element.",
          t,
          b
        ), g[t] = !0;
      switch (t) {
        case "dangerouslySetInnerHTML":
        case "children":
        case "style":
        case "suppressContentEditableWarning":
        case "suppressHydrationWarning":
        case "defaultValue":
        case "defaultChecked":
        case "innerHTML":
        case "ref":
          return !0;
        case "innerText":
        case "textContent":
          return !0;
      }
      switch (typeof c) {
        case "boolean":
          switch (t) {
            case "autoFocus":
            case "checked":
            case "multiple":
            case "muted":
            case "selected":
            case "contentEditable":
            case "spellCheck":
            case "draggable":
            case "value":
            case "autoReverse":
            case "externalResourcesRequired":
            case "focusable":
            case "preserveAlpha":
            case "allowFullScreen":
            case "async":
            case "autoPlay":
            case "controls":
            case "default":
            case "defer":
            case "disabled":
            case "disablePictureInPicture":
            case "disableRemotePlayback":
            case "formNoValidate":
            case "hidden":
            case "loop":
            case "noModule":
            case "noValidate":
            case "open":
            case "playsInline":
            case "readOnly":
            case "required":
            case "reversed":
            case "scoped":
            case "seamless":
            case "itemScope":
            case "capture":
            case "download":
            case "inert":
              return !0;
            default:
              return b = t.toLowerCase().slice(0, 5), b === "data-" || b === "aria-" ? !0 : (c ? console.error(
                'Received `%s` for a non-boolean attribute `%s`.\n\nIf you want to write it to the DOM, pass a string instead: %s="%s" or %s={value.toString()}.',
                c,
                t,
                t,
                c,
                t
              ) : console.error(
                'Received `%s` for a non-boolean attribute `%s`.\n\nIf you want to write it to the DOM, pass a string instead: %s="%s" or %s={value.toString()}.\n\nIf you used to conditionally omit it with %s={condition && value}, pass %s={condition ? value : undefined} instead.',
                c,
                t,
                t,
                c,
                t,
                t,
                t
              ), g[t] = !0);
          }
        case "function":
        case "symbol":
          return g[t] = !0, !1;
        case "string":
          if (c === "false" || c === "true") {
            switch (t) {
              case "checked":
              case "selected":
              case "multiple":
              case "muted":
              case "allowFullScreen":
              case "async":
              case "autoPlay":
              case "controls":
              case "default":
              case "defer":
              case "disabled":
              case "disablePictureInPicture":
              case "disableRemotePlayback":
              case "formNoValidate":
              case "hidden":
              case "loop":
              case "noModule":
              case "noValidate":
              case "open":
              case "playsInline":
              case "readOnly":
              case "required":
              case "reversed":
              case "scoped":
              case "seamless":
              case "itemScope":
              case "inert":
                break;
              default:
                return !0;
            }
            console.error(
              "Received the string `%s` for the boolean attribute `%s`. %s Did you mean %s={%s}?",
              c,
              t,
              c === "false" ? "The browser will interpret it as a truthy value." : 'Although this works, it will not work as expected if you pass the string "false".',
              t,
              c
            ), g[t] = !0;
          }
      }
      return !0;
    }
    function Li(e, t, c) {
      var d = [], b;
      for (b in t)
        eo(e, b, t[b]) || d.push(b);
      t = d.map(function(w) {
        return "`" + w + "`";
      }).join(", "), d.length === 1 ? console.error(
        "Invalid value for prop %s on <%s> tag. Either remove it from the element, or pass a string or number value to keep it in the DOM. For details, see https://react.dev/link/attribute-behavior ",
        t,
        e
      ) : 1 < d.length && console.error(
        "Invalid values for props %s on <%s> tag. Either remove them from the element, or pass a string or number value to keep them in the DOM. For details, see https://react.dev/link/attribute-behavior ",
        t,
        e
      );
    }
    function Wt(e) {
      return e.replace(q, function(t, c) {
        return c.toUpperCase();
      });
    }
    function ge(e) {
      if (typeof e == "boolean" || typeof e == "number" || typeof e == "bigint")
        return "" + e;
      Ir(e), e = "" + e;
      var t = xe.exec(e);
      if (t) {
        var c = "", d, b = 0;
        for (d = t.index; d < e.length; d++) {
          switch (e.charCodeAt(d)) {
            case 34:
              t = "&quot;";
              break;
            case 38:
              t = "&amp;";
              break;
            case 39:
              t = "&#x27;";
              break;
            case 60:
              t = "&lt;";
              break;
            case 62:
              t = "&gt;";
              break;
            default:
              continue;
          }
          b !== d && (c += e.slice(b, d)), b = d + 1, c += t;
        }
        e = b !== d ? c + e.slice(b, d) : c;
      }
      return e;
    }
    function V(e) {
      return _n.test("" + e) ? "javascript:throw new Error('React has blocked a javascript: URL as a security precaution.')" : e;
    }
    function ue(e) {
      return Ir(e), ("" + e).replace(Aa, Vn);
    }
    function Wl(e, t, c, d, b, w) {
      c = typeof t == "string" ? t : t && t.script;
      var k = c === void 0 ? qi : N(
        '<script nonce="' + ge(c) + '"'
      ), A = typeof t == "string" ? void 0 : t && t.style, X = A === void 0 ? wt : N(
        '<style nonce="' + ge(A) + '"'
      ), O = e.idPrefix, z = [], ee = e.bootstrapScriptContent, re = e.bootstrapScripts, te = e.bootstrapModules;
      if (ee !== void 0 && (z.push(k), ll(z, e), z.push(
        Un,
        ne(
          ue(ee)
        ),
        Ei
      )), ee = [], d !== void 0 && (ee.push(Ia), ee.push(
        ne(
          ue(JSON.stringify(d))
        )
      ), ee.push(lc)), b && typeof w == "number" && 0 >= w && console.error(
        "React expected a positive non-zero `maxHeadersLength` option but found %s instead. When using the `onHeaders` option you may supply an optional `maxHeadersLength` option as well however, when setting this value to zero or less no headers will be captured.",
        w === 0 ? "zero" : w
      ), d = b ? {
        preconnects: "",
        fontPreloads: "",
        highImagePreloads: "",
        remainingCapacity: 2 + (typeof w == "number" ? w : 2e3)
      } : null, b = {
        placeholderPrefix: N(O + "P:"),
        segmentPrefix: N(O + "S:"),
        boundaryPrefix: N(O + "B:"),
        startInlineScript: k,
        startInlineStyle: X,
        preamble: le(),
        externalRuntimeScript: null,
        bootstrapChunks: z,
        importMapChunks: ee,
        onHeaders: b,
        headers: d,
        resets: {
          font: {},
          dns: {},
          connect: { default: {}, anonymous: {}, credentials: {} },
          image: {},
          style: {}
        },
        charsetChunks: [],
        viewportChunks: [],
        hoistableChunks: [],
        preconnects: /* @__PURE__ */ new Set(),
        fontPreloads: /* @__PURE__ */ new Set(),
        highImagePreloads: /* @__PURE__ */ new Set(),
        styles: /* @__PURE__ */ new Map(),
        bootstrapScripts: /* @__PURE__ */ new Set(),
        scripts: /* @__PURE__ */ new Set(),
        bulkPreloads: /* @__PURE__ */ new Set(),
        preloads: {
          images: /* @__PURE__ */ new Map(),
          stylesheets: /* @__PURE__ */ new Map(),
          scripts: /* @__PURE__ */ new Map(),
          moduleScripts: /* @__PURE__ */ new Map()
        },
        nonce: { script: c, style: A },
        hoistableState: null,
        stylesToHoist: !1
      }, re !== void 0)
        for (d = 0; d < re.length; d++)
          w = re[d], X = A = void 0, O = {
            rel: "preload",
            as: "script",
            fetchPriority: "low",
            nonce: t
          }, typeof w == "string" ? O.href = k = w : (O.href = k = w.src, O.integrity = X = typeof w.integrity == "string" ? w.integrity : void 0, O.crossOrigin = A = typeof w == "string" || w.crossOrigin == null ? void 0 : w.crossOrigin === "use-credentials" ? "use-credentials" : ""), qn(
            e,
            b,
            k,
            O
          ), z.push(
            tr,
            ne(ge(k)),
            ze
          ), c && z.push(
            al,
            ne(ge(c)),
            ze
          ), typeof X == "string" && z.push(
            ol,
            ne(ge(X)),
            ze
          ), typeof A == "string" && z.push(
            ql,
            ne(ge(A)),
            ze
          ), ll(z, e), z.push(Ri);
      if (te !== void 0)
        for (t = 0; t < te.length; t++)
          re = te[t], k = w = void 0, A = {
            rel: "modulepreload",
            fetchPriority: "low",
            nonce: c
          }, typeof re == "string" ? A.href = d = re : (A.href = d = re.src, A.integrity = k = typeof re.integrity == "string" ? re.integrity : void 0, A.crossOrigin = w = typeof re == "string" || re.crossOrigin == null ? void 0 : re.crossOrigin === "use-credentials" ? "use-credentials" : ""), qn(
            e,
            b,
            d,
            A
          ), z.push(
            il,
            ne(ge(d)),
            ze
          ), c && z.push(
            al,
            ne(ge(c)),
            ze
          ), typeof k == "string" && z.push(
            ol,
            ne(ge(k)),
            ze
          ), typeof w == "string" && z.push(
            ql,
            ne(ge(w)),
            ze
          ), ll(z, e), z.push(Ri);
      return b;
    }
    function no(e, t, c, d, b) {
      return {
        idPrefix: e === void 0 ? "" : e,
        nextFormID: 0,
        streamingFormat: 0,
        bootstrapScriptContent: c,
        bootstrapScripts: d,
        bootstrapModules: b,
        instructions: Hn,
        hasBody: !1,
        hasHtml: !1,
        unknownResources: {},
        dnsResources: {},
        connectResources: { default: {}, anonymous: {}, credentials: {} },
        imageResources: {},
        styleResources: {},
        scriptResources: {},
        moduleUnknownResources: {},
        moduleScriptResources: {}
      };
    }
    function le() {
      return { htmlChunks: null, headChunks: null, bodyChunks: null };
    }
    function F(e, t, c, d) {
      return {
        insertionMode: e,
        selectedValue: t,
        tagScope: c,
        viewTransition: d
      };
    }
    function de(e) {
      return F(
        e === "http://www.w3.org/2000/svg" ? lr : e === "http://www.w3.org/1998/Math/MathML" ? $r : bo,
        null,
        0,
        null
      );
    }
    function We(e, t, c) {
      var d = e.tagScope & -25;
      switch (t) {
        case "noscript":
          return F(rr, null, d | 1, null);
        case "select":
          return F(
            rr,
            c.value != null ? c.value : c.defaultValue,
            d,
            null
          );
        case "svg":
          return F(lr, null, d, null);
        case "picture":
          return F(rr, null, d | 2, null);
        case "math":
          return F($r, null, d, null);
        case "foreignObject":
          return F(rr, null, d, null);
        case "table":
          return F(Ci, null, d, null);
        case "thead":
        case "tbody":
        case "tfoot":
          return F(
            ac,
            null,
            d,
            null
          );
        case "colgroup":
          return F(
            Uc,
            null,
            d,
            null
          );
        case "tr":
          return F(
            ki,
            null,
            d,
            null
          );
        case "head":
          if (e.insertionMode < rr)
            return F(
              ic,
              null,
              d,
              null
            );
          break;
        case "html":
          if (e.insertionMode === bo)
            return F(
              ji,
              null,
              d,
              null
            );
      }
      return e.insertionMode >= Ci || e.insertionMode < rr ? F(rr, null, d, null) : e.tagScope !== d ? F(
        e.insertionMode,
        e.selectedValue,
        d,
        null
      ) : e;
    }
    function Ze(e) {
      return e === null ? null : {
        update: e.update,
        enter: "none",
        exit: "none",
        share: e.update,
        name: e.autoName,
        autoName: e.autoName,
        nameIdx: 0
      };
    }
    function He(e, t) {
      return t.tagScope & 32 && (e.instructions |= 128), F(
        t.insertionMode,
        t.selectedValue,
        t.tagScope | 12,
        Ze(t.viewTransition)
      );
    }
    function Ct(e, t) {
      e = Ze(t.viewTransition);
      var c = t.tagScope | 16;
      return e !== null && e.share !== "none" && (c |= 64), F(
        t.insertionMode,
        t.selectedValue,
        c,
        e
      );
    }
    function _e(e, t, c, d) {
      return t === "" ? d : (d && e.push(ct), e.push(ne(ge(t))), !0);
    }
    function pl(e, t) {
      if (typeof t != "object")
        throw Error(
          "The `style` prop expects a mapping from style properties to values, not a string. For example, style={{marginRight: spacing + 'em'}} when using JSX."
        );
      var c = !0, d;
      for (d in t)
        if (gn.call(t, d)) {
          var b = t[d];
          if (b != null && typeof b != "boolean" && b !== "") {
            if (d.indexOf("--") === 0) {
              var w = ne(ge(d));
              Ee(b, d), b = ne(
                ge(("" + b).trim())
              );
            } else {
              w = d;
              var k = b;
              if (-1 < w.indexOf("-")) {
                var A = w;
                ae.hasOwnProperty(A) && ae[A] || (ae[A] = !0, console.error(
                  "Unsupported style property %s. Did you mean %s?",
                  A,
                  Wt(A.replace(W, "ms-"))
                ));
              } else if (I.test(w))
                A = w, ae.hasOwnProperty(A) && ae[A] || (ae[A] = !0, console.error(
                  "Unsupported vendor-prefixed style property %s. Did you mean %s?",
                  A,
                  A.charAt(0).toUpperCase() + A.slice(1)
                ));
              else if (Q.test(k)) {
                A = w;
                var X = k;
                ve.hasOwnProperty(X) && ve[X] || (ve[X] = !0, console.error(
                  `Style property values shouldn't contain a semicolon. Try "%s: %s" instead.`,
                  A,
                  X.replace(
                    Q,
                    ""
                  )
                ));
              }
              typeof k == "number" && (isNaN(k) ? De || (De = !0, console.error(
                "`NaN` is an invalid value for the `%s` css style property.",
                w
              )) : isFinite(k) || ke || (ke = !0, console.error(
                "`Infinity` is an invalid value for the `%s` css style property.",
                w
              ))), w = d, k = oc.get(w), k !== void 0 || (k = N(
                ge(
                  w.replace(me, "-$1").toLowerCase().replace(Ce, "-ms-")
                )
              ), oc.set(w, k)), w = k, typeof b == "number" ? b = b === 0 || vo.has(d) ? ne("" + b) : ne(b + "px") : (Ee(b, d), b = ne(
                ge(("" + b).trim())
              ));
            }
            c ? (c = !1, e.push(
              $i,
              w,
              yo,
              b
            )) : e.push(Ma, w, yo, b);
          }
        }
      c || e.push(ze);
    }
    function Mr(e, t, c) {
      c && typeof c != "function" && typeof c != "symbol" && e.push(
        nt,
        ne(t),
        Si
      );
    }
    function Mt(e, t, c) {
      typeof c != "function" && typeof c != "symbol" && typeof c != "boolean" && e.push(
        nt,
        ne(t),
        el,
        ne(ge(c)),
        ze
      );
    }
    function rt(e, t) {
      this.push(ea), Sc(e), Mt(this, "name", t), Mt(this, "value", e), this.push(Cr);
    }
    function Sc(e) {
      if (typeof e != "string")
        throw Error(
          "File/Blob fields are not yet supported in progressive forms. Will fallback to client hydration."
        );
    }
    function Fe(e, t) {
      if (typeof t.$$FORM_ACTION == "function") {
        var c = e.nextFormID++;
        e = e.idPrefix + c;
        try {
          var d = t.$$FORM_ACTION(e);
          if (d) {
            var b = d.data;
            b != null && b.forEach(Sc);
          }
          return d;
        } catch (w) {
          if (typeof w == "object" && w !== null && typeof w.then == "function")
            throw w;
          console.error(
            `Failed to serialize an action for progressive enhancement:
%s`,
            w
          );
        }
      }
      return null;
    }
    function lt(e, t, c, d, b, w, k, A) {
      var X = null;
      if (typeof d == "function") {
        A === null || cc || (cc = !0, console.error(
          'Cannot specify a "name" prop for a button that specifies a function as a formAction. React needs it to encode which action should be invoked. It will get overridden.'
        )), b === null && w === null || Gc || (Gc = !0, console.error(
          "Cannot specify a formEncType or formMethod for a button that specifies a function as a formAction. React provides those automatically. They will get overridden."
        )), k === null || Yc || (Yc = !0, console.error(
          "Cannot specify a formTarget for a button that specifies a function as a formAction. The function will always be executed in the same window."
        ));
        var O = Fe(t, d);
        O !== null ? (A = O.name, d = O.action || "", b = O.encType, w = O.method, k = O.target, X = O.data) : (e.push(
          nt,
          ne("formAction"),
          el,
          xo,
          ze
        ), k = w = b = d = A = null, Ot(t, c));
      }
      return A != null && un(e, "name", A), d != null && un(e, "formAction", d), b != null && un(e, "formEncType", b), w != null && un(e, "formMethod", w), k != null && un(e, "formTarget", k), X;
    }
    function un(e, t, c) {
      switch (t) {
        case "className":
          Mt(e, "class", c);
          break;
        case "tabIndex":
          Mt(e, "tabindex", c);
          break;
        case "dir":
        case "role":
        case "viewBox":
        case "width":
        case "height":
          Mt(e, t, c);
          break;
        case "style":
          pl(e, c);
          break;
        case "src":
        case "href":
          if (c === "") {
            console.error(
              t === "src" ? 'An empty string ("") was passed to the %s attribute. This may cause the browser to download the whole page again over the network. To fix this, either do not render the element at all or pass null to %s instead of an empty string.' : 'An empty string ("") was passed to the %s attribute. To fix this, either do not render the element at all or pass null to %s instead of an empty string.',
              t,
              t
            );
            break;
          }
        case "action":
        case "formAction":
          if (c == null || typeof c == "function" || typeof c == "symbol" || typeof c == "boolean")
            break;
          Nt(c, t), c = V("" + c), e.push(
            nt,
            ne(t),
            el,
            ne(ge(c)),
            ze
          );
          break;
        case "defaultValue":
        case "defaultChecked":
        case "innerHTML":
        case "suppressContentEditableWarning":
        case "suppressHydrationWarning":
        case "ref":
          break;
        case "autoFocus":
        case "multiple":
        case "muted":
          Mr(e, t.toLowerCase(), c);
          break;
        case "xlinkHref":
          if (typeof c == "function" || typeof c == "symbol" || typeof c == "boolean")
            break;
          Nt(c, t), c = V("" + c), e.push(
            nt,
            ne("xlink:href"),
            el,
            ne(ge(c)),
            ze
          );
          break;
        case "contentEditable":
        case "spellCheck":
        case "draggable":
        case "value":
        case "autoReverse":
        case "externalResourcesRequired":
        case "focusable":
        case "preserveAlpha":
          typeof c != "function" && typeof c != "symbol" && e.push(
            nt,
            ne(t),
            el,
            ne(ge(c)),
            ze
          );
          break;
        case "inert":
          c !== "" || Hc[t] || (Hc[t] = !0, console.error(
            "Received an empty string for a boolean attribute `%s`. This will treat the attribute as if it were false. Either pass `false` to silence this warning, or pass `true` if you used an empty string in earlier versions of React to indicate this attribute is true.",
            t
          ));
        case "allowFullScreen":
        case "async":
        case "autoPlay":
        case "controls":
        case "default":
        case "defer":
        case "disabled":
        case "disablePictureInPicture":
        case "disableRemotePlayback":
        case "formNoValidate":
        case "hidden":
        case "loop":
        case "noModule":
        case "noValidate":
        case "open":
        case "playsInline":
        case "readOnly":
        case "required":
        case "reversed":
        case "scoped":
        case "seamless":
        case "itemScope":
          c && typeof c != "function" && typeof c != "symbol" && e.push(
            nt,
            ne(t),
            Si
          );
          break;
        case "capture":
        case "download":
          c === !0 ? e.push(
            nt,
            ne(t),
            Si
          ) : c !== !1 && typeof c != "function" && typeof c != "symbol" && e.push(
            nt,
            ne(t),
            el,
            ne(ge(c)),
            ze
          );
          break;
        case "cols":
        case "rows":
        case "size":
        case "span":
          typeof c != "function" && typeof c != "symbol" && !isNaN(c) && 1 <= c && e.push(
            nt,
            ne(t),
            el,
            ne(ge(c)),
            ze
          );
          break;
        case "rowSpan":
        case "start":
          typeof c == "function" || typeof c == "symbol" || isNaN(c) || e.push(
            nt,
            ne(t),
            el,
            ne(ge(c)),
            ze
          );
          break;
        case "xlinkActuate":
          Mt(e, "xlink:actuate", c);
          break;
        case "xlinkArcrole":
          Mt(e, "xlink:arcrole", c);
          break;
        case "xlinkRole":
          Mt(e, "xlink:role", c);
          break;
        case "xlinkShow":
          Mt(e, "xlink:show", c);
          break;
        case "xlinkTitle":
          Mt(e, "xlink:title", c);
          break;
        case "xlinkType":
          Mt(e, "xlink:type", c);
          break;
        case "xmlBase":
          Mt(e, "xml:base", c);
          break;
        case "xmlLang":
          Mt(e, "xml:lang", c);
          break;
        case "xmlSpace":
          Mt(e, "xml:space", c);
          break;
        default:
          if ((!(2 < t.length) || t[0] !== "o" && t[0] !== "O" || t[1] !== "n" && t[1] !== "N") && (t = Pu.get(t) || t, Rt(t))) {
            switch (typeof c) {
              case "function":
              case "symbol":
                return;
              case "boolean":
                var d = t.toLowerCase().slice(0, 5);
                if (d !== "data-" && d !== "aria-") return;
            }
            e.push(
              nt,
              ne(t),
              el,
              ne(ge(c)),
              ze
            );
          }
      }
    }
    function kt(e, t, c) {
      if (t != null) {
        if (c != null)
          throw Error(
            "Can only set one of `children` or `props.dangerouslySetInnerHTML`."
          );
        if (typeof t != "object" || !("__html" in t))
          throw Error(
            "`props.dangerouslySetInnerHTML` must be in the form `{__html: ...}`. Please visit https://react.dev/link/dangerously-set-inner-html for more information."
          );
        t = t.__html, t != null && (Ir(t), e.push(ne("" + t)));
      }
    }
    function da(e, t) {
      var c = e[t];
      c != null && (c = Kl(c), e.multiple && !c ? console.error(
        "The `%s` prop supplied to <select> must be an array if `multiple` is true.",
        t
      ) : !e.multiple && c && console.error(
        "The `%s` prop supplied to <select> must be a scalar value if `multiple` is false.",
        t
      ));
    }
    function zi(e) {
      var t = "";
      return Lc.Children.forEach(e, function(c) {
        c != null && (t += c, To || typeof c == "string" || typeof c == "number" || typeof c == "bigint" || (To = !0, console.error(
          "Cannot infer the option value of complex children. Pass a `value` prop or use a plain string as children to <option>."
        )));
      }), t;
    }
    function Ot(e, t) {
      if ((e.instructions & 16) === Hn) {
        e.instructions |= 16;
        var c = t.preamble, d = t.bootstrapChunks;
        (c.htmlChunks || c.headChunks) && d.length === 0 ? (d.push(t.startInlineScript), ll(d, e), d.push(
          Un,
          Da,
          Ei
        )) : d.unshift(
          t.startInlineScript,
          Un,
          Da,
          Ei
        );
      }
    }
    function xr(e, t) {
      e.push(Rn("link"));
      for (var c in t)
        if (gn.call(t, c)) {
          var d = t[c];
          if (d != null)
            switch (c) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(
                  "link is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                );
              default:
                un(e, c, d);
            }
        }
      return e.push(Cr), null;
    }
    function El(e) {
      return Ir(e), ("" + e).replace(uc, Te);
    }
    function yt(e, t, c) {
      e.push(Rn(c));
      for (var d in t)
        if (gn.call(t, d)) {
          var b = t[d];
          if (b != null)
            switch (d) {
              case "children":
              case "dangerouslySetInnerHTML":
                throw Error(
                  c + " is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                );
              default:
                un(e, d, b);
            }
        }
      return e.push(Cr), null;
    }
    function Rl(e, t) {
      e.push(Rn("title"));
      var c = null, d = null, b;
      for (b in t)
        if (gn.call(t, b)) {
          var w = t[b];
          if (w != null)
            switch (b) {
              case "children":
                c = w;
                break;
              case "dangerouslySetInnerHTML":
                d = w;
                break;
              default:
                un(e, b, w);
            }
        }
      return e.push(Un), t = Array.isArray(c) ? 2 > c.length ? c[0] : null : c, typeof t != "function" && typeof t != "symbol" && t !== null && t !== void 0 && e.push(ne(ge("" + t))), kt(e, d, c), e.push(et("title")), null;
    }
    function Pc(e, t) {
      e.push(Rn("script"));
      var c = null, d = null, b;
      for (b in t)
        if (gn.call(t, b)) {
          var w = t[b];
          if (w != null)
            switch (b) {
              case "children":
                c = w;
                break;
              case "dangerouslySetInnerHTML":
                d = w;
                break;
              default:
                un(e, b, w);
            }
        }
      return e.push(Un), c != null && typeof c != "string" && (t = typeof c == "number" ? "a number for children" : Array.isArray(c) ? "an array for children" : "something unexpected for children", console.error(
        "A script element was rendered with %s. If script element has children it must be a single string. Consider using dangerouslySetInnerHTML or passing a plain string as children.",
        t
      )), kt(e, d, c), typeof c == "string" && e.push(ne(ue(c))), e.push(et("script")), null;
    }
    function qe(e, t, c) {
      e.push(Rn(c));
      var d = c = null, b;
      for (b in t)
        if (gn.call(t, b)) {
          var w = t[b];
          if (w != null)
            switch (b) {
              case "children":
                c = w;
                break;
              case "dangerouslySetInnerHTML":
                d = w;
                break;
              default:
                un(e, b, w);
            }
        }
      return e.push(Un), kt(e, d, c), c;
    }
    function $t(e, t, c) {
      e.push(Rn(c));
      var d = c = null, b;
      for (b in t)
        if (gn.call(t, b)) {
          var w = t[b];
          if (w != null)
            switch (b) {
              case "children":
                c = w;
                break;
              case "dangerouslySetInnerHTML":
                d = w;
                break;
              default:
                un(e, b, w);
            }
        }
      return e.push(Un), kt(e, d, c), typeof c == "string" ? (e.push(ne(ge(c))), null) : c;
    }
    function Rn(e) {
      var t = Mu.get(e);
      if (t === void 0) {
        if (!Iu.test(e)) throw Error("Invalid tag: " + e);
        t = N("<" + e), Mu.set(e, t);
      }
      return t;
    }
    function Xu(e, t, c, d, b, w, k, A, X) {
      kc(t, c), t !== "input" && t !== "textarea" && t !== "select" || c == null || c.value !== null || o || (o = !0, t === "select" && c.multiple ? console.error(
        "`value` prop on `%s` should not be null. Consider using an empty array when `multiple` is set to `true` to clear the component or `undefined` for uncontrolled components.",
        t
      ) : console.error(
        "`value` prop on `%s` should not be null. Consider using an empty string to clear the component or `undefined` for uncontrolled components.",
        t
      ));
      e: if (t.indexOf("-") === -1) var O = !1;
      else
        switch (t) {
          case "annotation-xml":
          case "color-profile":
          case "font-face":
          case "font-face-src":
          case "font-face-uri":
          case "font-face-format":
          case "font-face-name":
          case "missing-glyph":
            O = !1;
            break e;
          default:
            O = !0;
        }
      switch (O || typeof c.is == "string" || Li(t, c), !c.suppressContentEditableWarning && c.contentEditable && c.children != null && console.error(
        "A component is `contentEditable` and contains `children` managed by React. It is now your responsibility to guarantee that none of those nodes are unexpectedly modified or duplicated. This is probably not intentional."
      ), A.insertionMode !== lr && A.insertionMode !== $r && t.indexOf("-") === -1 && t.toLowerCase() !== t && console.error(
        "<%s /> is using incorrect casing. Use PascalCase for React components, or lowercase for HTML elements.",
        t
      ), t) {
        case "div":
        case "span":
        case "svg":
        case "path":
          break;
        case "a":
          e.push(Rn("a"));
          var z = null, ee = null, re;
          for (re in c)
            if (gn.call(c, re)) {
              var te = c[re];
              if (te != null)
                switch (re) {
                  case "children":
                    z = te;
                    break;
                  case "dangerouslySetInnerHTML":
                    ee = te;
                    break;
                  case "href":
                    te === "" ? Mt(e, "href", "") : un(e, re, te);
                    break;
                  default:
                    un(e, re, te);
                }
            }
          if (e.push(Un), kt(e, ee, z), typeof z == "string") {
            e.push(ne(ge(z)));
            var j = null;
          } else j = z;
          return j;
        case "g":
        case "p":
        case "li":
          break;
        case "select":
          dn("select", c), da(c, "value"), da(c, "defaultValue"), c.value === void 0 || c.defaultValue === void 0 || cl || (console.error(
            "Select elements must be either controlled or uncontrolled (specify either the value prop, or the defaultValue prop, but not both). Decide between using a controlled or uncontrolled select element and remove one of these props. More info: https://react.dev/link/controlled-components"
          ), cl = !0), e.push(Rn("select"));
          var we = null, pn = null, ye;
          for (ye in c)
            if (gn.call(c, ye)) {
              var Ue = c[ye];
              if (Ue != null)
                switch (ye) {
                  case "children":
                    we = Ue;
                    break;
                  case "dangerouslySetInnerHTML":
                    pn = Ue;
                    break;
                  case "defaultValue":
                  case "value":
                    break;
                  default:
                    un(
                      e,
                      ye,
                      Ue
                    );
                }
            }
          return e.push(Un), kt(e, pn, we), we;
        case "option":
          var jn = A.selectedValue;
          e.push(Rn("option"));
          var ft = null, an = null, be = null, dt = null, hr;
          for (hr in c)
            if (gn.call(c, hr)) {
              var En = c[hr];
              if (En != null)
                switch (hr) {
                  case "children":
                    ft = En;
                    break;
                  case "selected":
                    be = En, Fu || (console.error(
                      "Use the `defaultValue` or `value` props on <select> instead of setting `selected` on <option>."
                    ), Fu = !0);
                    break;
                  case "dangerouslySetInnerHTML":
                    dt = En;
                    break;
                  case "value":
                    an = En;
                  default:
                    un(
                      e,
                      hr,
                      En
                    );
                }
            }
          if (jn != null) {
            if (an !== null) {
              Nt(an, "value");
              var zn = "" + an;
            } else
              dt === null || ir || (ir = !0, console.error(
                "Pass a `value` prop if you set dangerouslyInnerHTML so React knows which value should be selected."
              )), zn = zi(ft);
            if (Kl(jn)) {
              for (var Nr = 0; Nr < jn.length; Nr++)
                if (Nt(jn[Nr], "value"), "" + jn[Nr] === zn) {
                  e.push(jl);
                  break;
                }
            } else
              Nt(jn, "select.value"), "" + jn === zn && e.push(jl);
          } else be && e.push(jl);
          return e.push(Un), kt(e, dt, ft), ft;
        case "textarea":
          dn("textarea", c), c.value === void 0 || c.defaultValue === void 0 || Xt || (console.error(
            "Textarea elements must be either controlled or uncontrolled (specify either the value prop, or the defaultValue prop, but not both). Decide between using a controlled or uncontrolled textarea and remove one of these props. More info: https://react.dev/link/controlled-components"
          ), Xt = !0), e.push(Rn("textarea"));
          var Kt = null, At = null, Bn = null, pt;
          for (pt in c)
            if (gn.call(c, pt)) {
              var Wr = c[pt];
              if (Wr != null)
                switch (pt) {
                  case "children":
                    Bn = Wr;
                    break;
                  case "value":
                    Kt = Wr;
                    break;
                  case "defaultValue":
                    At = Wr;
                    break;
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "`dangerouslySetInnerHTML` does not make sense on <textarea>."
                    );
                  default:
                    un(
                      e,
                      pt,
                      Wr
                    );
                }
            }
          if (Kt === null && At !== null && (Kt = At), e.push(Un), Bn != null) {
            if (console.error(
              "Use the `defaultValue` or `value` props instead of setting children on <textarea>."
            ), Kt != null)
              throw Error(
                "If you supply `defaultValue` on a <textarea>, do not pass children."
              );
            if (Kl(Bn)) {
              if (1 < Bn.length)
                throw Error("<textarea> can only have at most one child.");
              Ir(Bn[0]), Kt = "" + Bn[0];
            }
            Ir(Bn), Kt = "" + Bn;
          }
          return typeof Kt == "string" && Kt[0] === `
` && e.push(Xc), Kt !== null && (Nt(Kt, "value"), e.push(
            ne(ge("" + Kt))
          )), null;
        case "input":
          dn("input", c), e.push(Rn("input"));
          var rl = null, It = null, Zn = null, gr = null, Hr = null, vr = null, Ii = null, Ur = null, ht = null, vl;
          for (vl in c)
            if (gn.call(c, vl)) {
              var Bt = c[vl];
              if (Bt != null)
                switch (vl) {
                  case "children":
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "input is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                    );
                  case "name":
                    rl = Bt;
                    break;
                  case "formAction":
                    It = Bt;
                    break;
                  case "formEncType":
                    Zn = Bt;
                    break;
                  case "formMethod":
                    gr = Bt;
                    break;
                  case "formTarget":
                    Hr = Bt;
                    break;
                  case "defaultChecked":
                    ht = Bt;
                    break;
                  case "defaultValue":
                    Ii = Bt;
                    break;
                  case "checked":
                    Ur = Bt;
                    break;
                  case "value":
                    vr = Bt;
                    break;
                  default:
                    un(
                      e,
                      vl,
                      Bt
                    );
                }
            }
          It === null || c.type === "image" || c.type === "submit" || _a || (_a = !0, console.error(
            'An input can only specify a formAction along with type="submit" or type="image".'
          ));
          var Bu = lt(
            e,
            d,
            b,
            It,
            Zn,
            gr,
            Hr,
            rl
          );
          return Ur === null || ht === null || Oa || (console.error(
            "%s contains an input of type %s with both checked and defaultChecked props. Input elements must be either controlled or uncontrolled (specify either the checked prop, or the defaultChecked prop, but not both). Decide between using a controlled or uncontrolled input element and remove one of these props. More info: https://react.dev/link/controlled-components",
            "A component",
            c.type
          ), Oa = !0), vr === null || Ii === null || na || (console.error(
            "%s contains an input of type %s with both value and defaultValue props. Input elements must be either controlled or uncontrolled (specify either the value prop, or the defaultValue prop, but not both). Decide between using a controlled or uncontrolled input element and remove one of these props. More info: https://react.dev/link/controlled-components",
            "A component",
            c.type
          ), na = !0), Ur !== null ? Mr(e, "checked", Ur) : ht !== null && Mr(e, "checked", ht), vr !== null ? un(e, "value", vr) : Ii !== null && un(e, "value", Ii), e.push(Cr), Bu != null && Bu.forEach(rt, e), null;
        case "button":
          e.push(Rn("button"));
          var ii = null, $n = null, bl = null, br = null, xc = null, Io = null, zl = null, Mo;
          for (Mo in c)
            if (gn.call(c, Mo)) {
              var Yr = c[Mo];
              if (Yr != null)
                switch (Mo) {
                  case "children":
                    ii = Yr;
                    break;
                  case "dangerouslySetInnerHTML":
                    $n = Yr;
                    break;
                  case "name":
                    bl = Yr;
                    break;
                  case "formAction":
                    br = Yr;
                    break;
                  case "formEncType":
                    xc = Yr;
                    break;
                  case "formMethod":
                    Io = Yr;
                    break;
                  case "formTarget":
                    zl = Yr;
                    break;
                  default:
                    un(
                      e,
                      Mo,
                      Yr
                    );
                }
            }
          br === null || c.type == null || c.type === "submit" || _a || (_a = !0, console.error(
            'A button can only specify a formAction along with type="submit" or no type.'
          ));
          var Tc = lt(
            e,
            d,
            b,
            br,
            xc,
            Io,
            zl,
            bl
          );
          if (e.push(Un), Tc != null && Tc.forEach(rt, e), kt(e, $n, ii), typeof ii == "string") {
            e.push(
              ne(ge(ii))
            );
            var Nu = null;
          } else Nu = ii;
          return Nu;
        case "form":
          e.push(Rn("form"));
          var yl = null, wc = null, yr = null, Oo = null, Va = null, ua = null, xl;
          for (xl in c)
            if (gn.call(c, xl)) {
              var Tl = c[xl];
              if (Tl != null)
                switch (xl) {
                  case "children":
                    yl = Tl;
                    break;
                  case "dangerouslySetInnerHTML":
                    wc = Tl;
                    break;
                  case "action":
                    yr = Tl;
                    break;
                  case "encType":
                    Oo = Tl;
                    break;
                  case "method":
                    Va = Tl;
                    break;
                  case "target":
                    ua = Tl;
                    break;
                  default:
                    un(
                      e,
                      xl,
                      Tl
                    );
                }
            }
          var ai = null, Gr = null;
          if (typeof yr == "function") {
            Oo === null && Va === null || Gc || (Gc = !0, console.error(
              "Cannot specify a encType or method for a form that specifies a function as the action. React provides those automatically. They will get overridden."
            )), ua === null || Yc || (Yc = !0, console.error(
              "Cannot specify a target for a form that specifies a function as the action. The function will always be executed in the same window."
            ));
            var qt = Fe(
              d,
              yr
            );
            qt !== null ? (yr = qt.action || "", Oo = qt.encType, Va = qt.method, ua = qt.target, ai = qt.data, Gr = qt.name) : (e.push(
              nt,
              ne("action"),
              el,
              xo,
              ze
            ), ua = Va = Oo = yr = null, Ot(d, b));
          }
          if (yr != null && un(e, "action", yr), Oo != null && un(e, "encType", Oo), Va != null && un(e, "method", Va), ua != null && un(e, "target", ua), e.push(Un), Gr !== null && (e.push(ea), Mt(e, "name", Gr), e.push(Cr), ai != null && ai.forEach(
            rt,
            e
          )), kt(e, wc, yl), typeof yl == "string") {
            e.push(
              ne(ge(yl))
            );
            var sa = null;
          } else sa = yl;
          return sa;
        case "menuitem":
          e.push(Rn("menuitem"));
          for (var oi in c)
            if (gn.call(c, oi)) {
              var Qa = c[oi];
              if (Qa != null)
                switch (oi) {
                  case "children":
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "menuitems cannot have `children` nor `dangerouslySetInnerHTML`."
                    );
                  default:
                    un(
                      e,
                      oi,
                      Qa
                    );
                }
            }
          return e.push(Un), null;
        case "object":
          e.push(Rn("object"));
          var Mi = null, ds = null, Fr;
          for (Fr in c)
            if (gn.call(c, Fr)) {
              var Bl = c[Fr];
              if (Bl != null)
                switch (Fr) {
                  case "children":
                    Mi = Bl;
                    break;
                  case "dangerouslySetInnerHTML":
                    ds = Bl;
                    break;
                  case "data":
                    Nt(Bl, "data");
                    var pc = V("" + Bl);
                    if (pc === "") {
                      console.error(
                        'An empty string ("") was passed to the %s attribute. To fix this, either do not render the element at all or pass null to %s instead of an empty string.',
                        Fr,
                        Fr
                      );
                      break;
                    }
                    e.push(
                      nt,
                      ne("data"),
                      el,
                      ne(ge(pc)),
                      ze
                    );
                    break;
                  default:
                    un(
                      e,
                      Fr,
                      Bl
                    );
                }
            }
          if (e.push(Un), kt(e, ds, Mi), typeof Mi == "string") {
            e.push(
              ne(ge(Mi))
            );
            var au = null;
          } else au = Mi;
          return au;
        case "title":
          var ou = A.tagScope & 1, zs = A.tagScope & 4;
          if (gn.call(c, "children")) {
            var Ka = c.children, _o = Array.isArray(Ka) ? 2 > Ka.length ? Ka[0] : null : Ka;
            Array.isArray(Ka) && 1 < Ka.length ? console.error(
              "React expects the `children` prop of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found an Array with length %s instead. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert `children` of <title> tags to a single string value which is why Arrays of length greater than 1 are not supported. When using JSX it can be common to combine text nodes and value nodes. For example: <title>hello {nameOfUser}</title>. While not immediately apparent, `children` in this case is an Array with length 2. If your `children` prop is using this form try rewriting it using a template string: <title>{`hello ${nameOfUser}`}</title>.",
              Ka.length
            ) : typeof _o == "function" || typeof _o == "symbol" ? console.error(
              "React expect children of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found %s instead. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert children of <title> tags to a single string value.",
              typeof _o == "function" ? "a Function" : "a Sybmol"
            ) : _o && _o.toString === {}.toString && (_o.$$typeof != null ? console.error(
              "React expects the `children` prop of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found an object that appears to be a React element which never implements a suitable `toString` method. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert children of <title> tags to a single string value which is why rendering React elements is not supported. If the `children` of <title> is a React Component try moving the <title> tag into that component. If the `children` of <title> is some HTML markup change it to be Text only to be valid HTML."
            ) : console.error(
              "React expects the `children` prop of <title> tags to be a string, number, bigint, or object with a novel `toString` method but found an object that does not implement a suitable `toString` method. Browsers treat all child Nodes of <title> tags as Text content and React expects to be able to convert children of <title> tags to a single string value. Using the default `toString` method available on every object is almost certainly an error. Consider whether the `children` of this <title> is an object in error and change it to a string or number value if so. Otherwise implement a `toString` method that React can use to produce a valid <title>."
            ));
          }
          if (A.insertionMode === lr || ou || c.itemProp != null)
            var qa = Rl(
              e,
              c
            );
          else
            zs ? qa = null : (Rl(b.hoistableChunks, c), qa = void 0);
          return qa;
        case "link":
          var cu = A.tagScope & 1, ms = A.tagScope & 4, ws = c.rel, mr = c.href, ci = c.precedence;
          if (A.insertionMode === lr || cu || c.itemProp != null || typeof ws != "string" || typeof mr != "string" || mr === "") {
            ws === "stylesheet" && typeof c.precedence == "string" && (typeof mr == "string" && mr || console.error(
              'React encountered a `<link rel="stylesheet" .../>` with a `precedence` prop and expected the `href` prop to be a non-empty string but ecountered %s instead. If your intent was to have React hoist and deduplciate this stylesheet using the `precedence` prop ensure there is a non-empty string `href` prop as well, otherwise remove the `precedence` prop.',
              mr === null ? "`null`" : mr === void 0 ? "`undefined`" : mr === "" ? "an empty string" : 'something with type "' + typeof mr + '"'
            )), xr(e, c);
            var ja = null;
          } else if (c.rel === "stylesheet")
            if (typeof ci != "string" || c.disabled != null || c.onLoad || c.onError) {
              if (typeof ci == "string") {
                if (c.disabled != null)
                  console.error(
                    'React encountered a `<link rel="stylesheet" .../>` with a `precedence` prop and a `disabled` prop. The presence of the `disabled` prop indicates an intent to manage the stylesheet active state from your from your Component code and React will not hoist or deduplicate this stylesheet. If your intent was to have React hoist and deduplciate this stylesheet using the `precedence` prop remove the `disabled` prop, otherwise remove the `precedence` prop.'
                  );
                else if (c.onLoad || c.onError) {
                  var Wu = c.onLoad && c.onError ? "`onLoad` and `onError` props" : c.onLoad ? "`onLoad` prop" : "`onError` prop";
                  console.error(
                    'React encountered a `<link rel="stylesheet" .../>` with a `precedence` prop and %s. The presence of loading and error handlers indicates an intent to manage the stylesheet loading state from your from your Component code and React will not hoist or deduplicate this stylesheet. If your intent was to have React hoist and deduplciate this stylesheet using the `precedence` prop remove the %s, otherwise remove the `precedence` prop.',
                    Wu,
                    Wu
                  );
                }
              }
              ja = xr(
                e,
                c
              );
            } else {
              var wl = b.styles.get(ci), gt = d.styleResources.hasOwnProperty(
                mr
              ) ? d.styleResources[mr] : void 0;
              if (gt !== rn) {
                d.styleResources[mr] = rn, wl || (wl = {
                  precedence: ne(ge(ci)),
                  rules: [],
                  hrefs: [],
                  sheets: /* @__PURE__ */ new Map()
                }, b.styles.set(ci, wl));
                var Jn = {
                  state: aa,
                  props: Pn({}, c, {
                    "data-precedence": c.precedence,
                    precedence: null
                  })
                };
                if (gt) {
                  gt.length === 2 && Ni(Jn.props, gt);
                  var mn = b.preloads.stylesheets.get(mr);
                  mn && 0 < mn.length ? mn.length = 0 : Jn.state = $c;
                }
                wl.sheets.set(mr, Jn), k && k.stylesheets.add(Jn);
              } else if (wl) {
                var ps = wl.sheets.get(mr);
                ps && k && k.stylesheets.add(ps);
              }
              X && e.push(ct), ja = null;
            }
          else
            c.onLoad || c.onError ? ja = xr(
              e,
              c
            ) : (X && e.push(ct), ja = ms ? null : xr(b.hoistableChunks, c));
          return ja;
        case "script":
          var Do = A.tagScope & 1, Hu = c.async;
          if (typeof c.src != "string" || !c.src || !Hu || typeof Hu == "function" || typeof Hu == "symbol" || c.onLoad || c.onError || A.insertionMode === lr || Do || c.itemProp != null)
            var Lo = Pc(
              e,
              c
            );
          else {
            var Ec = c.src;
            if (c.type === "module")
              var Uu = d.moduleScriptResources, Rc = b.preloads.moduleScripts;
            else
              Uu = d.scriptResources, Rc = b.preloads.scripts;
            var uu = Uu.hasOwnProperty(Ec) ? Uu[Ec] : void 0;
            if (uu !== rn) {
              Uu[Ec] = rn;
              var su = c;
              if (uu) {
                uu.length === 2 && (su = Pn({}, c), Ni(su, uu));
                var hs = Rc.get(Ec);
                hs && (hs.length = 0);
              }
              var Es = [];
              b.scripts.add(Es), Pc(Es, su);
            }
            X && e.push(ct), Lo = null;
          }
          return Lo;
        case "style":
          var As = A.tagScope & 1;
          if (gn.call(c, "children")) {
            var Yu = c.children, Gu = Array.isArray(Yu) ? 2 > Yu.length ? Yu[0] : null : Yu;
            (typeof Gu == "function" || typeof Gu == "symbol" || Array.isArray(Gu)) && console.error(
              "React expect children of <style> tags to be a string, number, or object with a `toString` method but found %s instead. In browsers style Elements can only have `Text` Nodes as children.",
              typeof Gu == "function" ? "a Function" : typeof Gu == "symbol" ? "a Sybmol" : "an Array"
            );
          }
          var Cc = c.precedence, $a = c.href, fa = c.nonce;
          if (A.insertionMode === lr || As || c.itemProp != null || typeof Cc != "string" || typeof $a != "string" || $a === "") {
            e.push(Rn("style"));
            var Xr = null, fu = null, Is;
            for (Is in c)
              if (gn.call(c, Is)) {
                var Bs = c[Is];
                if (Bs != null)
                  switch (Is) {
                    case "children":
                      Xr = Bs;
                      break;
                    case "dangerouslySetInnerHTML":
                      fu = Bs;
                      break;
                    default:
                      un(
                        e,
                        Is,
                        Bs
                      );
                  }
              }
            e.push(Un);
            var Rs = Array.isArray(Xr) ? 2 > Xr.length ? Xr[0] : null : Xr;
            typeof Rs != "function" && typeof Rs != "symbol" && Rs !== null && Rs !== void 0 && e.push(
              ne(El(Rs))
            ), kt(
              e,
              fu,
              Xr
            ), e.push(et("style"));
            var js = null;
          } else {
            $a.includes(" ") && console.error(
              'React expected the `href` prop for a <style> tag opting into hoisting semantics using the `precedence` prop to not have any spaces but ecountered spaces instead. using spaces in this prop will cause hydration of this style to fail on the client. The href for the <style> where this ocurred is "%s".',
              $a
            );
            var gs = b.styles.get(Cc), $s = d.styleResources.hasOwnProperty($a) ? d.styleResources[$a] : void 0;
            if ($s !== rn) {
              d.styleResources[$a] = rn, $s && console.error(
                'React encountered a hoistable style tag for the same href as a preload: "%s". When using a style tag to inline styles you should not also preload it as a stylsheet.',
                $a
              ), gs || (gs = {
                precedence: ne(
                  ge(Cc)
                ),
                rules: [],
                hrefs: [],
                sheets: /* @__PURE__ */ new Map()
              }, b.styles.set(
                Cc,
                gs
              ));
              var Ms = b.nonce.style;
              if (Ms && Ms !== fa)
                console.error(
                  'React encountered a style tag with `precedence` "%s" and `nonce` "%s". When React manages style rules using `precedence` it will only include rules if the nonce matches the style nonce "%s" that was included with this render.',
                  Cc,
                  fa,
                  Ms
                );
              else {
                !Ms && fa && console.error(
                  'React encountered a style tag with `precedence` "%s" and `nonce` "%s". When React manages style rules using `precedence` it will only include a nonce attributes if you also provide the same style nonce value as a render option.',
                  Cc,
                  fa
                ), gs.hrefs.push(
                  ne(ge($a))
                );
                var Ns = gs.rules, Ws = null, bf = null, ef;
                for (ef in c)
                  if (gn.call(c, ef)) {
                    var lf = c[ef];
                    if (lf != null)
                      switch (ef) {
                        case "children":
                          Ws = lf;
                          break;
                        case "dangerouslySetInnerHTML":
                          bf = lf;
                      }
                  }
                var Ys = Array.isArray(Ws) ? 2 > Ws.length ? Ws[0] : null : Ws;
                typeof Ys != "function" && typeof Ys != "symbol" && Ys !== null && Ys !== void 0 && Ns.push(
                  ne(El(Ys))
                ), kt(Ns, bf, Ws);
              }
            }
            gs && k && k.styles.add(gs), X && e.push(ct), js = void 0;
          }
          return js;
        case "meta":
          var If = A.tagScope & 1, Mf = A.tagScope & 4;
          if (A.insertionMode === lr || If || c.itemProp != null)
            var yf = yt(
              e,
              c,
              "meta"
            );
          else
            X && e.push(ct), yf = Mf ? null : typeof c.charSet == "string" ? yt(b.charsetChunks, c, "meta") : c.name === "viewport" ? yt(b.viewportChunks, c, "meta") : yt(
              b.hoistableChunks,
              c,
              "meta"
            );
          return yf;
        case "listing":
        case "pre":
          e.push(Rn(t));
          var Gs = null, Xs = null, Zs;
          for (Zs in c)
            if (gn.call(c, Zs)) {
              var nf = c[Zs];
              if (nf != null)
                switch (Zs) {
                  case "children":
                    Gs = nf;
                    break;
                  case "dangerouslySetInnerHTML":
                    Xs = nf;
                    break;
                  default:
                    un(
                      e,
                      Zs,
                      nf
                    );
                }
            }
          if (e.push(Un), Xs != null) {
            if (Gs != null)
              throw Error(
                "Can only set one of `children` or `props.dangerouslySetInnerHTML`."
              );
            if (typeof Xs != "object" || !("__html" in Xs))
              throw Error(
                "`props.dangerouslySetInnerHTML` must be in the form `{__html: ...}`. Please visit https://react.dev/link/dangerously-set-inner-html for more information."
              );
            var Cs = Xs.__html;
            Cs != null && (typeof Cs == "string" && 0 < Cs.length && Cs[0] === `
` ? e.push(Xc, ne(Cs)) : (Ir(Cs), e.push(ne("" + Cs))));
          }
          return typeof Gs == "string" && Gs[0] === `
` && e.push(Xc), Gs;
        case "img":
          var Of = A.tagScope & 3, Oi = c.src, ui = c.srcSet;
          if (!(c.loading === "lazy" || !Oi && !ui || typeof Oi != "string" && Oi != null || typeof ui != "string" && ui != null || c.fetchPriority === "low" || Of) && (typeof Oi != "string" || Oi[4] !== ":" || Oi[0] !== "d" && Oi[0] !== "D" || Oi[1] !== "a" && Oi[1] !== "A" || Oi[2] !== "t" && Oi[2] !== "T" || Oi[3] !== "a" && Oi[3] !== "A") && (typeof ui != "string" || ui[4] !== ":" || ui[0] !== "d" && ui[0] !== "D" || ui[1] !== "a" && ui[1] !== "A" || ui[2] !== "t" && ui[2] !== "T" || ui[3] !== "a" && ui[3] !== "A")) {
            k !== null && A.tagScope & 64 && (k.suspenseyImages = !0);
            var xf = typeof c.sizes == "string" ? c.sizes : void 0, Hs = ui ? ui + `
` + (xf || "") : Oi, af = b.preloads.images, Os = af.get(Hs);
            if (Os)
              (c.fetchPriority === "high" || 10 > b.highImagePreloads.size) && (af.delete(Hs), b.highImagePreloads.add(Os));
            else if (!d.imageResources.hasOwnProperty(Hs)) {
              d.imageResources[Hs] = Ft;
              var of = c.crossOrigin, Tf = typeof of == "string" ? of === "use-credentials" ? of : "" : void 0, _s = b.headers, cf;
              _s && 0 < _s.remainingCapacity && typeof c.srcSet != "string" && (c.fetchPriority === "high" || 500 > _s.highImagePreloads.length) && (cf = Wi(Oi, "image", {
                imageSrcSet: c.srcSet,
                imageSizes: c.sizes,
                crossOrigin: Tf,
                integrity: c.integrity,
                nonce: c.nonce,
                type: c.type,
                fetchPriority: c.fetchPriority,
                referrerPolicy: c.refererPolicy
              }), 0 <= (_s.remainingCapacity -= cf.length + 2)) ? (b.resets.image[Hs] = Ft, _s.highImagePreloads && (_s.highImagePreloads += ", "), _s.highImagePreloads += cf) : (Os = [], xr(Os, {
                rel: "preload",
                as: "image",
                href: ui ? void 0 : Oi,
                imageSrcSet: ui,
                imageSizes: xf,
                crossOrigin: Tf,
                integrity: c.integrity,
                type: c.type,
                fetchPriority: c.fetchPriority,
                referrerPolicy: c.referrerPolicy
              }), c.fetchPriority === "high" || 10 > b.highImagePreloads.size ? b.highImagePreloads.add(Os) : (b.bulkPreloads.add(Os), af.set(Hs, Os)));
            }
          }
          return yt(e, c, "img");
        case "base":
        case "area":
        case "br":
        case "col":
        case "embed":
        case "hr":
        case "keygen":
        case "param":
        case "source":
        case "track":
        case "wbr":
          return yt(e, c, t);
        case "annotation-xml":
        case "color-profile":
        case "font-face":
        case "font-face-src":
        case "font-face-uri":
        case "font-face-format":
        case "font-face-name":
        case "missing-glyph":
          break;
        case "head":
          if (A.insertionMode < rr) {
            var uf = w || b.preamble;
            if (uf.headChunks)
              throw Error("The `<head>` tag may only be rendered once.");
            w !== null && e.push(sc), uf.headChunks = [];
            var wf = qe(
              uf.headChunks,
              c,
              "head"
            );
          } else
            wf = $t(
              e,
              c,
              "head"
            );
          return wf;
        case "body":
          if (A.insertionMode < rr) {
            var sf = w || b.preamble;
            if (sf.bodyChunks)
              throw Error("The `<body>` tag may only be rendered once.");
            w !== null && e.push(mu), sf.bodyChunks = [];
            var pf = qe(
              sf.bodyChunks,
              c,
              "body"
            );
          } else
            pf = $t(
              e,
              c,
              "body"
            );
          return pf;
        case "html":
          if (A.insertionMode === bo) {
            var ff = w || b.preamble;
            if (ff.htmlChunks)
              throw Error("The `<html>` tag may only be rendered once.");
            w !== null && e.push(Au), ff.htmlChunks = [ut];
            var Ef = qe(
              ff.htmlChunks,
              c,
              "html"
            );
          } else
            Ef = $t(
              e,
              c,
              "html"
            );
          return Ef;
        default:
          if (t.indexOf("-") !== -1) {
            e.push(Rn(t));
            var df = null, Rf = null, Us;
            for (Us in c)
              if (gn.call(c, Us)) {
                var du = c[Us];
                if (du != null) {
                  var Cf = Us;
                  switch (Us) {
                    case "children":
                      df = du;
                      break;
                    case "dangerouslySetInnerHTML":
                      Rf = du;
                      break;
                    case "style":
                      pl(e, du);
                      break;
                    case "suppressContentEditableWarning":
                    case "suppressHydrationWarning":
                    case "ref":
                      break;
                    case "className":
                      Cf = "class";
                    default:
                      if (Rt(Us) && typeof du != "function" && typeof du != "symbol" && du !== !1) {
                        if (du === !0)
                          du = "";
                        else if (typeof du == "object")
                          continue;
                        e.push(
                          nt,
                          ne(Cf),
                          el,
                          ne(
                            ge(du)
                          ),
                          ze
                        );
                      }
                  }
                }
              }
            return e.push(Un), kt(
              e,
              Rf,
              df
            ), df;
          }
      }
      return $t(e, c, t);
    }
    function et(e) {
      var t = Zc.get(e);
      return t === void 0 && (t = N("</" + e + ">"), Zc.set(e, t)), t;
    }
    function hu(e, t) {
      e = e.preamble, e.htmlChunks === null && t.htmlChunks && (e.htmlChunks = t.htmlChunks), e.headChunks === null && t.headChunks && (e.headChunks = t.headChunks), e.bodyChunks === null && t.bodyChunks && (e.bodyChunks = t.bodyChunks);
    }
    function gu(e, t) {
      t = t.bootstrapChunks;
      for (var c = 0; c < t.length - 1; c++)
        H(e, t[c]);
      return c < t.length ? (c = t[c], t.length = 0, pe(e, c)) : !0;
    }
    function si(e, t, c) {
      if (H(e, ta), c === null)
        throw Error(
          "An ID must have been assigned before we can complete the boundary."
        );
      return H(e, t.boundaryPrefix), H(e, ne(c.toString(16))), pe(e, ul);
    }
    function vu(e, t, c, d) {
      switch (c.insertionMode) {
        case bo:
        case ji:
        case ic:
        case rr:
          return H(e, ls), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, Ol);
        case lr:
          return H(e, nl), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, ia);
        case $r:
          return H(e, sl), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, _u);
        case Ci:
          return H(e, Eo), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, l);
        case ac:
          return H(e, s), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, v);
        case ki:
          return H(e, E), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, C);
        case Uc:
          return H(e, m), H(e, t.segmentPrefix), H(e, ne(d.toString(16))), pe(e, D);
        default:
          throw Error("Unknown insertion mode. This is a bug in React.");
      }
    }
    function No(e, t) {
      switch (t.insertionMode) {
        case bo:
        case ji:
        case ic:
        case rr:
          return pe(e, po);
        case lr:
          return pe(e, Ba);
        case $r:
          return pe(e, is);
        case Ci:
          return pe(e, a);
        case ac:
          return pe(e, x);
        case ki:
          return pe(e, _);
        case Uc:
          return pe(e, U);
        default:
          throw Error("Unknown insertion mode. This is a bug in React.");
      }
    }
    function ha(e) {
      return JSON.stringify(e).replace(
        dl,
        function(t) {
          switch (t) {
            case "<":
              return "\\u003c";
            case "\u2028":
              return "\\u2028";
            case "\u2029":
              return "\\u2029";
            default:
              throw Error(
                "escapeJSStringsForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
              );
          }
        }
      );
    }
    function Bi(e) {
      return JSON.stringify(e).replace(
        cr,
        function(t) {
          switch (t) {
            case "&":
              return "\\u0026";
            case ">":
              return "\\u003e";
            case "<":
              return "\\u003c";
            case "\u2028":
              return "\\u2028";
            case "\u2029":
              return "\\u2029";
            default:
              throw Error(
                "escapeJSObjectForInstructionScripts encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
              );
          }
        }
      );
    }
    function ga(e) {
      var t = e.rules, c = e.hrefs;
      0 < t.length && c.length === 0 && console.error(
        "React expected to have at least one href for an a hoistable style but found none. This is a bug in React."
      );
      var d = 0;
      if (c.length) {
        for (H(this, Be.startInlineStyle), H(this, _l), H(this, e.precedence), H(this, ur); d < c.length - 1; d++)
          H(this, c[d]), H(this, Dn);
        for (H(this, c[d]), H(this, Wa), d = 0; d < t.length; d++) H(this, t[d]);
        Co = pe(
          this,
          Ro
        ), Sr = !0, t.length = 0, c.length = 0;
      }
    }
    function Wo(e) {
      return e.state !== $l ? Sr = !0 : !1;
    }
    function Nn(e, t, c) {
      return Sr = !1, Co = !0, Be = c, t.styles.forEach(ga, e), Be = null, t.stylesheets.forEach(Wo), Sr && (c.stylesToHoist = !0), Co;
    }
    function An(e) {
      for (var t = 0; t < e.length; t++) H(this, e[t]);
      e.length = 0;
    }
    function Fc(e) {
      xr(sr, e.props);
      for (var t = 0; t < sr.length; t++)
        H(this, sr[t]);
      sr.length = 0, e.state = $l;
    }
    function va(e) {
      var t = 0 < e.sheets.size;
      e.sheets.forEach(Fc, this), e.sheets.clear();
      var c = e.rules, d = e.hrefs;
      if (!t || d.length) {
        if (H(this, Be.startInlineStyle), H(this, Ha), H(this, e.precedence), e = 0, d.length) {
          for (H(this, Du); e < d.length - 1; e++)
            H(this, d[e]), H(this, Dn);
          H(this, d[e]);
        }
        for (H(this, ko), e = 0; e < c.length; e++)
          H(this, c[e]);
        H(this, Dl), c.length = 0, d.length = 0;
      }
    }
    function Zu(e) {
      if (e.state === aa) {
        e.state = $c;
        var t = e.props;
        for (xr(sr, {
          rel: "preload",
          as: "style",
          href: e.props.href,
          crossOrigin: t.crossOrigin,
          fetchPriority: t.fetchPriority,
          integrity: t.integrity,
          media: t.media,
          hrefLang: t.hrefLang,
          referrerPolicy: t.referrerPolicy
        }), e = 0; e < sr.length; e++)
          H(this, sr[e]);
        sr.length = 0;
      }
    }
    function bu(e) {
      e.sheets.forEach(Zu, this), e.sheets.clear();
    }
    function ll(e, t) {
      (t.instructions & tn) === Hn && (t.instructions |= tn, e.push(
        Kc,
        ne(
          ge("_" + t.idPrefix + "R_")
        ),
        ze
      ));
    }
    function Hl(e, t) {
      H(e, Ua);
      var c = Ua;
      t.stylesheets.forEach(function(d) {
        if (d.state !== $l)
          if (d.state === Ya)
            H(e, c), d = d.props.href, Nt(d, "href"), H(
              e,
              ne(
                Bi("" + d)
              )
            ), H(e, So), c = qc;
          else {
            H(e, c);
            var b = d.props["data-precedence"], w = d.props, k = V("" + d.props.href);
            H(
              e,
              ne(Bi(k))
            ), Nt(b, "precedence"), b = "" + b, H(e, jc), H(
              e,
              ne(Bi(b))
            );
            for (var A in w)
              if (gn.call(w, A) && (b = w[A], b != null))
                switch (A) {
                  case "href":
                  case "rel":
                  case "precedence":
                  case "data-precedence":
                    break;
                  case "children":
                  case "dangerouslySetInnerHTML":
                    throw Error(
                      "link is a self-closing tag and must neither have `children` nor use `dangerouslySetInnerHTML`."
                    );
                  default:
                    Ju(
                      e,
                      A,
                      b
                    );
                }
            H(e, So), c = qc, d.state = Ya;
          }
      }), H(e, So);
    }
    function Ju(e, t, c) {
      var d = t.toLowerCase();
      switch (typeof c) {
        case "function":
        case "symbol":
          return;
      }
      switch (t) {
        case "innerHTML":
        case "dangerouslySetInnerHTML":
        case "suppressContentEditableWarning":
        case "suppressHydrationWarning":
        case "style":
        case "ref":
          return;
        case "className":
          d = "class", Nt(c, d), t = "" + c;
          break;
        case "hidden":
          if (c === !1) return;
          t = "";
          break;
        case "src":
        case "href":
          c = V(c), Nt(c, d), t = "" + c;
          break;
        default:
          if (2 < t.length && (t[0] === "o" || t[0] === "O") && (t[1] === "n" || t[1] === "N") || !Rt(t))
            return;
          Nt(c, d), t = "" + c;
      }
      H(e, jc), H(
        e,
        ne(Bi(d))
      ), H(e, jc), H(
        e,
        ne(Bi(t))
      );
    }
    function ba() {
      return { styles: /* @__PURE__ */ new Set(), stylesheets: /* @__PURE__ */ new Set(), suspenseyImages: !1 };
    }
    function qn(e, t, c, d) {
      (e.scriptResources.hasOwnProperty(c) || e.moduleScriptResources.hasOwnProperty(c)) && console.error(
        'Internal React Error: React expected bootstrap script or module with src "%s" to not have been preloaded already. please file an issue',
        c
      ), e.scriptResources[c] = rn, e.moduleScriptResources[c] = rn, e = [], xr(e, d), t.bootstrapScripts.add(e);
    }
    function Ni(e, t) {
      e.crossOrigin == null && (e.crossOrigin = t[0]), e.integrity == null && (e.integrity = t[1]);
    }
    function Wi(e, t, c) {
      e = ya(e), t = Or(t, "as"), t = "<" + e + '>; rel=preload; as="' + t + '"';
      for (var d in c)
        gn.call(c, d) && (e = c[d], typeof e == "string" && (t += "; " + d.toLowerCase() + '="' + Or(
          e,
          d
        ) + '"'));
      return t;
    }
    function ya(e) {
      return Nt(e, "href"), ("" + e).replace(
        Ga,
        Ht
      );
    }
    function Ht(e) {
      switch (e) {
        case "<":
          return "%3C";
        case ">":
          return "%3E";
        case `
`:
          return "%0A";
        case "\r":
          return "%0D";
        default:
          throw Error(
            "escapeLinkHrefForHeaderContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
          );
      }
    }
    function Or(e, t) {
      return Bo(e) && (console.error(
        "The provided `%s` option is an unsupported type %s. This value must be coerced to a string before using it here.",
        t,
        _i(e)
      ), Di(e)), ("" + e).replace(
        as,
        xn
      );
    }
    function xn(e) {
      switch (e) {
        case '"':
          return "%22";
        case "'":
          return "%27";
        case ";":
          return "%3B";
        case ",":
          return "%2C";
        case `
`:
          return "%0A";
        case "\r":
          return "%0D";
        default:
          throw Error(
            "escapeStringForLinkHeaderQuotedParamValueContextReplacer encountered a match it does not know how to replace. this means the match regex and the replacement characters are no longer in sync. This is a bug in React"
          );
      }
    }
    function Ho(e) {
      this.styles.add(e);
    }
    function Jr(e) {
      this.stylesheets.add(e);
    }
    function je(e, t) {
      t.styles.forEach(Ho, e), t.stylesheets.forEach(Jr, e), t.suspenseyImages && (e.suspenseyImages = !0);
    }
    function Vu(e) {
      return 0 < e.stylesheets.size || e.suspenseyImages;
    }
    function Ye(e) {
      if (e == null) return null;
      if (typeof e == "function")
        return e.$$typeof === os ? null : e.displayName || e.name || null;
      if (typeof e == "string") return e;
      switch (e) {
        case Ti:
          return "Fragment";
        case tc:
          return "Profiler";
        case nc:
          return "StrictMode";
        case Vi:
          return "Suspense";
        case ka:
          return "SuspenseList";
        case _t:
          return "Activity";
      }
      if (typeof e == "object")
        switch (typeof e.tag == "number" && console.error(
          "Received an unexpected object in getComponentNameFromType(). This is likely a bug in React. Please file an issue."
        ), e.$$typeof) {
          case ec:
            return "Portal";
          case On:
            return e.displayName || "Context";
          case Rr:
            return (e._context.displayName || "Context") + ".Consumer";
          case en:
            var t = e.render;
            return e = e.displayName, e || (e = t.displayName || t.name || "", e = e !== "" ? "ForwardRef(" + e + ")" : "ForwardRef"), e;
          case Sa:
            return t = e.displayName || null, t !== null ? t : Ye(e.type) || "Memo";
          case Qi:
            t = e._payload, e = e._init;
            try {
              return Ye(e(t));
            } catch {
            }
        }
      return null;
    }
    function Uo(e, t) {
      if (e !== t) {
        e.context._currentValue = e.parentValue, e = e.parent;
        var c = t.parent;
        if (e === null) {
          if (c !== null)
            throw Error(
              "The stacks must reach the root at the same time. This is a bug in React."
            );
        } else {
          if (c === null)
            throw Error(
              "The stacks must reach the root at the same time. This is a bug in React."
            );
          Uo(e, c);
        }
        t.context._currentValue = t.value;
      }
    }
    function Yo(e) {
      e.context._currentValue = e.parentValue, e = e.parent, e !== null && Yo(e);
    }
    function Go(e) {
      var t = e.parent;
      t !== null && Go(t), e.context._currentValue = e.value;
    }
    function Ul(e, t) {
      if (e.context._currentValue = e.parentValue, e = e.parent, e === null)
        throw Error(
          "The depth must equal at least at zero before reaching the root. This is a bug in React."
        );
      e.depth === t.depth ? Uo(e, t) : Ul(e, t);
    }
    function hn(e, t) {
      var c = t.parent;
      if (c === null)
        throw Error(
          "The depth must equal at least at zero before reaching the root. This is a bug in React."
        );
      e.depth === c.depth ? Uo(e, c) : hn(e, c), t.context._currentValue = t.value;
    }
    function $e(e) {
      var t = Fi;
      t !== e && (t === null ? Go(e) : e === null ? Yo(t) : t.depth === e.depth ? Uo(t, e) : t.depth > e.depth ? Ul(t, e) : hn(t, e), Fi = e);
    }
    function Cl(e) {
      if (e !== null && typeof e != "function") {
        var t = String(e);
        zu.has(t) || (zu.add(t), console.error(
          "Expected the last optional `callback` argument to be a function. Instead received: %s.",
          e
        ));
      }
    }
    function Ut(e, t) {
      e = (e = e.constructor) && Ye(e) || "ReactClass";
      var c = e + "." + t;
      eu[c] || (console.error(
        `Can only update a mounting component. This usually means you called %s() outside componentWillMount() on the server. This is a no-op.

Please check the code for the %s component.`,
        t,
        e
      ), eu[c] = !0);
    }
    function xa(e, t, c) {
      var d = e.id;
      e = e.overflow;
      var b = 32 - Fo(d) - 1;
      d &= ~(1 << b), c += 1;
      var w = 32 - Fo(t) + b;
      if (30 < w) {
        var k = b - b % 5;
        return w = (d & (1 << k) - 1).toString(32), d >>= k, b -= k, {
          id: 1 << 32 - Fo(t) + b | c << b | d,
          overflow: w + e
        };
      }
      return {
        id: 1 << w | c << b | d,
        overflow: e
      };
    }
    function mc(e) {
      return e >>>= 0, e === 0 ? 32 : 31 - (Ss(e) / Ps | 0) | 0;
    }
    function er() {
    }
    function Qu(e, t, c) {
      switch (c = e[c], c === void 0 ? e.push(t) : c !== t && (t.then(er, er), t = c), t.status) {
        case "fulfilled":
          return t.value;
        case "rejected":
          throw t.reason;
        default:
          switch (typeof t.status == "string" ? t.then(er, er) : (e = t, e.status = "pending", e.then(
            function(d) {
              if (t.status === "pending") {
                var b = t;
                b.status = "fulfilled", b.value = d;
              }
            },
            function(d) {
              if (t.status === "pending") {
                var b = t;
                b.status = "rejected", b.reason = d;
              }
            }
          )), t.status) {
            case "fulfilled":
              return t.value;
            case "rejected":
              throw t.reason;
          }
          throw ss = t, ei;
      }
    }
    function yu() {
      if (ss === null)
        throw Error(
          "Expected a suspended thenable. This is a bug in React. Please file an issue."
        );
      var e = ss;
      return ss = null, e;
    }
    function Ac(e, t) {
      return e === t && (e !== 0 || 1 / e === 1 / t) || e !== e && t !== t;
    }
    function In() {
      if (Lt === null)
        throw Error(
          `Invalid hook call. Hooks can only be called inside of the body of a function component. This could happen for one of the following reasons:
1. You might have mismatching versions of React and the renderer (such as React DOM)
2. You might be breaking the Rules of Hooks
3. You might have more than one copy of React in the same app
See https://react.dev/link/invalid-hook-call for tips about how to debug and fix this problem.`
        );
      return Ll && console.error(
        "Do not call Hooks inside useEffect(...), useMemo(...), or other built-in Hooks. You can only call Hooks at the top level of your React function. For more information, see https://react.dev/link/rules-of-hooks"
      ), Lt;
    }
    function Ic() {
      if (0 < bc)
        throw Error("Rendered more hooks than during the previous render");
      return { memoizedState: null, queue: null, next: null };
    }
    function to() {
      return fn === null ? gl === null ? (Lr = !1, gl = fn = Ic()) : (Lr = !0, fn = gl) : fn.next === null ? (Lr = !1, fn = fn.next = Ic()) : (Lr = !0, fn = fn.next), fn;
    }
    function Vr() {
      var e = mo;
      return mo = null, e;
    }
    function Ta() {
      Ll = !1, ca = fr = mi = Lt = null, Ai = !1, gl = null, bc = 0, fn = Za = null;
    }
    function Ku(e) {
      return Ll && console.error(
        "Context can only be read while React is rendering. In classes, you can read it in the render method or getDerivedStateFromProps. In function components, you can read it directly in the function body, but not inside Hooks like useReducer() or useMemo()."
      ), e._currentValue;
    }
    function qu(e, t) {
      return typeof t == "function" ? t(e) : t;
    }
    function ro(e, t, c) {
      if (e !== qu && (Ja = "useReducer"), Lt = In(), fn = to(), Lr) {
        if (c = fn.queue, t = c.dispatch, Za !== null) {
          var d = Za.get(c);
          if (d !== void 0) {
            Za.delete(c), c = fn.memoizedState;
            do {
              var b = d.action;
              Ll = !0, c = e(c, b), Ll = !1, d = d.next;
            } while (d !== null);
            return fn.memoizedState = c, [c, t];
          }
        }
        return [fn.memoizedState, t];
      }
      return Ll = !0, e = e === qu ? typeof t == "function" ? t() : t : c !== void 0 ? c(t) : t, Ll = !1, fn.memoizedState = e, e = fn.queue = { last: null, dispatch: null }, e = e.dispatch = Yl.bind(
        null,
        Lt,
        e
      ), [fn.memoizedState, e];
    }
    function lo(e, t) {
      if (Lt = In(), fn = to(), t = t === void 0 ? null : t, fn !== null) {
        var c = fn.memoizedState;
        if (c !== null && t !== null) {
          e: {
            var d = c[1];
            if (d === null)
              console.error(
                "%s received a final argument during this render, but not during the previous render. Even though the final argument is optional, its type cannot change between renders.",
                Ja
              ), d = !1;
            else {
              t.length !== d.length && console.error(
                `The final argument passed to %s changed size between renders. The order and size of this array must remain constant.

Previous: %s
Incoming: %s`,
                Ja,
                "[" + t.join(", ") + "]",
                "[" + d.join(", ") + "]"
              );
              for (var b = 0; b < d.length && b < t.length; b++)
                if (!Fs(t[b], d[b])) {
                  d = !1;
                  break e;
                }
              d = !0;
            }
          }
          if (d) return c[0];
        }
      }
      return Ll = !0, e = e(), Ll = !1, fn.memoizedState = [e, t], e;
    }
    function Yl(e, t, c) {
      if (25 <= bc)
        throw Error(
          "Too many re-renders. React limits the number of renders to prevent an infinite loop."
        );
      if (e === Lt)
        if (Ai = !0, e = { action: c, next: null }, Za === null && (Za = /* @__PURE__ */ new Map()), c = Za.get(t), c === void 0)
          Za.set(t, e);
        else {
          for (t = c; t.next !== null; ) t = t.next;
          t.next = e;
        }
    }
    function Gl() {
      throw Error(
        "A function wrapped in useEffectEvent can't be called during rendering."
      );
    }
    function Mc() {
      throw Error("startTransition cannot be called during server rendering.");
    }
    function Xo() {
      throw Error("Cannot update optimistic state while rendering.");
    }
    function Zo(e, t, c) {
      In();
      var d = vc++, b = fr;
      if (typeof e.$$FORM_ACTION == "function") {
        var w = null, k = ca;
        b = b.formState;
        var A = e.$$IS_SIGNATURE_EQUAL;
        if (b !== null && typeof A == "function") {
          var X = b[1];
          A.call(e, b[2], b[3]) && (w = c !== void 0 ? "p" + c : "k" + Nl(
            JSON.stringify([
              k,
              null,
              d
            ]),
            0
          ), X === w && (iu = d, t = b[0]));
        }
        var O = e.bind(null, t);
        return e = function(ee) {
          O(ee);
        }, typeof O.$$FORM_ACTION == "function" && (e.$$FORM_ACTION = function(ee) {
          ee = O.$$FORM_ACTION(ee), c !== void 0 && (Nt(c, "target"), c += "", ee.action = c);
          var re = ee.data;
          return re && (w === null && (w = c !== void 0 ? "p" + c : "k" + Nl(
            JSON.stringify([
              k,
              null,
              d
            ]),
            0
          )), re.append("$ACTION_KEY", w)), ee;
        }), [t, e, !1];
      }
      var z = e.bind(null, t);
      return [
        t,
        function(ee) {
          z(ee);
        },
        !1
      ];
    }
    function xt(e) {
      var t = fs;
      return fs += 1, mo === null && (mo = []), Qu(mo, e, t);
    }
    function io() {
      throw Error("Cache cannot be refreshed during server rendering.");
    }
    function Jo() {
    }
    function Oc() {
      if (h === 0) {
        y = console.log, p = console.info, P = console.warn, M = console.error, K = console.group, L = console.groupCollapsed, J = console.groupEnd;
        var e = {
          configurable: !0,
          enumerable: !0,
          value: Jo,
          writable: !0
        };
        Object.defineProperties(console, {
          info: e,
          log: e,
          warn: e,
          error: e,
          group: e,
          groupCollapsed: e,
          groupEnd: e
        });
      }
      h++;
    }
    function Xl() {
      if (h--, h === 0) {
        var e = { configurable: !0, enumerable: !0, writable: !0 };
        Object.defineProperties(console, {
          log: Pn({}, e, { value: y }),
          info: Pn({}, e, { value: p }),
          warn: Pn({}, e, { value: P }),
          error: Pn({}, e, { value: M }),
          group: Pn({}, e, { value: K }),
          groupCollapsed: Pn({}, e, { value: L }),
          groupEnd: Pn({}, e, { value: J })
        });
      }
      0 > h && console.error(
        "disabledDepth fell below zero. This is a bug in React. Please file an issue."
      );
    }
    function Vo(e) {
      var t = Error.prepareStackTrace;
      if (Error.prepareStackTrace = void 0, e = e.stack, Error.prepareStackTrace = t, e.startsWith(`Error: react-stack-top-frame
`) && (e = e.slice(29)), t = e.indexOf(`
`), t !== -1 && (e = e.slice(t + 1)), t = e.indexOf("react_stack_bottom_frame"), t !== -1 && (t = e.lastIndexOf(
        `
`,
        t
      )), t !== -1)
        e = e.slice(0, t);
      else return "";
      return e;
    }
    function Tr(e) {
      if (ie === void 0)
        try {
          throw Error();
        } catch (c) {
          var t = c.stack.trim().match(/\n( *(at )?)/);
          ie = t && t[1] || "", fe = -1 < c.stack.indexOf(`
    at`) ? " (<anonymous>)" : -1 < c.stack.indexOf("@") ? "@unknown:0:0" : "";
        }
      return `
` + ie + e + fe;
    }
    function kl(e, t) {
      if (!e || oe) return "";
      var c = $.get(e);
      if (c !== void 0) return c;
      oe = !0, c = Error.prepareStackTrace, Error.prepareStackTrace = void 0;
      var d = null;
      d = Le.H, Le.H = null, Oc();
      try {
        var b = {
          DetermineComponentFrameRoot: function() {
            try {
              if (t) {
                var re = function() {
                  throw Error();
                };
                if (Object.defineProperty(re.prototype, "props", {
                  set: function() {
                    throw Error();
                  }
                }), typeof Reflect == "object" && Reflect.construct) {
                  try {
                    Reflect.construct(re, []);
                  } catch (j) {
                    var te = j;
                  }
                  Reflect.construct(e, [], re);
                } else {
                  try {
                    re.call();
                  } catch (j) {
                    te = j;
                  }
                  e.call(re.prototype);
                }
              } else {
                try {
                  throw Error();
                } catch (j) {
                  te = j;
                }
                (re = e()) && typeof re.catch == "function" && re.catch(function() {
                });
              }
            } catch (j) {
              if (j && te && typeof j.stack == "string")
                return [j.stack, te.stack];
            }
            return [null, null];
          }
        };
        b.DetermineComponentFrameRoot.displayName = "DetermineComponentFrameRoot";
        var w = Object.getOwnPropertyDescriptor(
          b.DetermineComponentFrameRoot,
          "name"
        );
        w && w.configurable && Object.defineProperty(
          b.DetermineComponentFrameRoot,
          "name",
          { value: "DetermineComponentFrameRoot" }
        );
        var k = b.DetermineComponentFrameRoot(), A = k[0], X = k[1];
        if (A && X) {
          var O = A.split(`
`), z = X.split(`
`);
          for (k = w = 0; w < O.length && !O[w].includes(
            "DetermineComponentFrameRoot"
          ); )
            w++;
          for (; k < z.length && !z[k].includes(
            "DetermineComponentFrameRoot"
          ); )
            k++;
          if (w === O.length || k === z.length)
            for (w = O.length - 1, k = z.length - 1; 1 <= w && 0 <= k && O[w] !== z[k]; )
              k--;
          for (; 1 <= w && 0 <= k; w--, k--)
            if (O[w] !== z[k]) {
              if (w !== 1 || k !== 1)
                do
                  if (w--, k--, 0 > k || O[w] !== z[k]) {
                    var ee = `
` + O[w].replace(
                      " at new ",
                      " at "
                    );
                    return e.displayName && ee.includes("<anonymous>") && (ee = ee.replace("<anonymous>", e.displayName)), typeof e == "function" && $.set(e, ee), ee;
                  }
                while (1 <= w && 0 <= k);
              break;
            }
        }
      } finally {
        oe = !1, Le.H = d, Xl(), Error.prepareStackTrace = c;
      }
      return O = (O = e ? e.displayName || e.name : "") ? Tr(O) : "", typeof e == "function" && $.set(e, O), O;
    }
    function xu(e) {
      if (typeof e == "string") return Tr(e);
      if (typeof e == "function")
        return e.prototype && e.prototype.isReactComponent ? kl(e, !0) : kl(e, !1);
      if (typeof e == "object" && e !== null) {
        switch (e.$$typeof) {
          case en:
            return kl(e.render, !1);
          case Sa:
            return kl(e.type, !1);
          case Qi:
            var t = e, c = t._payload;
            t = t._init;
            try {
              e = t(c);
            } catch {
              return Tr("Lazy");
            }
            return xu(e);
        }
        if (typeof e.name == "string") {
          e: {
            if (c = e.name, t = e.env, e = e.debugLocation, e != null) {
              e = Vo(e);
              var d = e.lastIndexOf(`
`);
              if (e = d === -1 ? e : e.slice(d + 1), e.indexOf(c) !== -1) {
                c = `
` + e;
                break e;
              }
            }
            c = Tr(
              c + (t ? " [" + t + "]" : "")
            );
          }
          return c;
        }
      }
      switch (e) {
        case ka:
          return Tr("SuspenseList");
        case Vi:
          return Tr("Suspense");
      }
      return "";
    }
    function Qo() {
      var e = ni();
      1e3 < e - cn && (Le.recentlyCreatedOwnerStacks = 0, cn = e);
    }
    function wr(e, t) {
      return (500 < t.byteSize || Vu(t.contentState)) && t.contentPreamble === null;
    }
    function Ko(e) {
      if (typeof e == "object" && e !== null && typeof e.environmentName == "string") {
        var t = e.environmentName;
        e = [e].slice(0), typeof e[0] == "string" ? e.splice(
          0,
          1,
          "%c%s%c " + e[0],
          "background: #e6e6e6;background: light-dark(rgba(0,0,0,0.1), rgba(255,255,255,0.25));color: #000000;color: light-dark(#000000, #ffffff);border-radius: 2px",
          " " + t + " ",
          ""
        ) : e.splice(
          0,
          0,
          "%c%s%c",
          "background: #e6e6e6;background: light-dark(rgba(0,0,0,0.1), rgba(255,255,255,0.25));color: #000000;color: light-dark(#000000, #ffffff);border-radius: 2px",
          " " + t + " ",
          ""
        ), e.unshift(console), t = hl.apply(console.error, e), t();
      } else console.error(e);
      return null;
    }
    function Hi(e, t, c, d, b, w, k, A, X, O, z) {
      var ee = /* @__PURE__ */ new Set();
      this.destination = null, this.flushScheduled = !1, this.resumableState = e, this.renderState = t, this.rootFormatContext = c, this.progressiveChunkSize = d === void 0 ? 12800 : d, this.status = 10, this.fatalError = null, this.pendingRootTasks = this.allPendingTasks = this.nextSegmentId = 0, this.completedPreambleSegments = this.completedRootSegment = null, this.byteSize = 0, this.abortableTasks = ee, this.pingedTasks = [], this.clientRenderedBoundaries = [], this.completedBoundaries = [], this.partialBoundaries = [], this.trackedPostpones = null, this.onError = b === void 0 ? Ko : b, this.onPostpone = O === void 0 ? er : O, this.onAllReady = w === void 0 ? er : w, this.onShellReady = k === void 0 ? er : k, this.onShellError = A === void 0 ? er : A, this.onFatalError = X === void 0 ? er : X, this.formState = z === void 0 ? null : z, this.didWarnForKey = null;
    }
    function ao(e, t, c, d, b, w, k, A, X, O, z, ee) {
      return Qo(), t = new Hi(
        t,
        c,
        d,
        b,
        w,
        k,
        A,
        X,
        O,
        z,
        ee
      ), c = di(
        t,
        0,
        null,
        d,
        !1,
        !1
      ), c.parentFlushed = !0, e = Sl(
        t,
        null,
        e,
        -1,
        null,
        c,
        null,
        null,
        t.abortableTasks,
        null,
        d,
        null,
        lu,
        null,
        null,
        tl,
        null
      ), gi(e), t.pingedTasks.push(e), t;
    }
    function pr(e, t, c, d, b, w, k, A, X, O, z) {
      return e = ao(
        e,
        t,
        c,
        d,
        b,
        w,
        k,
        A,
        X,
        O,
        z,
        void 0
      ), e.trackedPostpones = {
        workingMap: /* @__PURE__ */ new Map(),
        rootNodes: [],
        rootSlots: null
      }, e;
    }
    function St(e, t, c, d, b, w, k, A, X) {
      return Qo(), c = new Hi(
        t.resumableState,
        c,
        t.rootFormatContext,
        t.progressiveChunkSize,
        d,
        b,
        w,
        k,
        A,
        X,
        null
      ), c.nextSegmentId = t.nextSegmentId, typeof t.replaySlots == "number" ? (d = di(
        c,
        0,
        null,
        t.rootFormatContext,
        !1,
        !1
      ), d.parentFlushed = !0, e = Sl(
        c,
        null,
        e,
        -1,
        null,
        d,
        null,
        null,
        c.abortableTasks,
        null,
        t.rootFormatContext,
        null,
        lu,
        null,
        null,
        tl,
        null
      ), gi(e), c.pingedTasks.push(e), c) : (e = wu(
        c,
        null,
        {
          nodes: t.replayNodes,
          slots: t.replaySlots,
          pendingTasks: 0
        },
        e,
        -1,
        null,
        null,
        c.abortableTasks,
        null,
        t.rootFormatContext,
        null,
        lu,
        null,
        null,
        tl,
        null
      ), gi(e), c.pingedTasks.push(e), c);
    }
    function Sn(e, t, c, d, b, w, k, A, X) {
      return e = St(
        e,
        t,
        c,
        d,
        b,
        w,
        k,
        A,
        X
      ), e.trackedPostpones = {
        workingMap: /* @__PURE__ */ new Map(),
        rootNodes: [],
        rootSlots: null
      }, e;
    }
    function Tu(e, t) {
      e.pingedTasks.push(t), e.pingedTasks.length === 1 && (e.flushScheduled = e.destination !== null, e.trackedPostpones !== null || e.status === 10 ? Fa(function() {
        return Ji(e);
      }) : Et(function() {
        return Ji(e);
      }));
    }
    function fi(e, t, c, d, b) {
      return c = {
        status: vn,
        rootSegmentID: -1,
        parentFlushed: !1,
        pendingTasks: 0,
        row: t,
        completedSegments: [],
        byteSize: 0,
        fallbackAbortableTasks: c,
        errorDigest: null,
        contentState: ba(),
        fallbackState: ba(),
        contentPreamble: d,
        fallbackPreamble: b,
        trackedContentKeyPath: null,
        trackedFallbackNode: null,
        errorMessage: null,
        errorStack: null,
        errorComponentStack: null
      }, t !== null && (t.pendingTasks++, d = t.boundaries, d !== null && (e.allPendingTasks++, c.pendingTasks++, d.push(c)), e = t.inheritedHoistables, e !== null && je(c.contentState, e)), c;
    }
    function Sl(e, t, c, d, b, w, k, A, X, O, z, ee, re, te, j, we, pn) {
      e.allPendingTasks++, b === null ? e.pendingRootTasks++ : b.pendingTasks++, te !== null && te.pendingTasks++;
      var ye = {
        replay: null,
        node: c,
        childIndex: d,
        ping: function() {
          return Tu(e, ye);
        },
        blockedBoundary: b,
        blockedSegment: w,
        blockedPreamble: k,
        hoistableState: A,
        abortSet: X,
        keyPath: O,
        formatContext: z,
        context: ee,
        treeContext: re,
        row: te,
        componentStack: j,
        thenableState: t
      };
      return ye.debugTask = pn, X.add(ye), ye;
    }
    function wu(e, t, c, d, b, w, k, A, X, O, z, ee, re, te, j, we) {
      e.allPendingTasks++, w === null ? e.pendingRootTasks++ : w.pendingTasks++, re !== null && re.pendingTasks++, c.pendingTasks++;
      var pn = {
        replay: c,
        node: d,
        childIndex: b,
        ping: function() {
          return Tu(e, pn);
        },
        blockedBoundary: w,
        blockedSegment: null,
        blockedPreamble: null,
        hoistableState: k,
        abortSet: A,
        keyPath: X,
        formatContext: O,
        context: z,
        treeContext: ee,
        row: re,
        componentStack: te,
        thenableState: t
      };
      return pn.debugTask = we, A.add(pn), pn;
    }
    function di(e, t, c, d, b, w) {
      return {
        status: vn,
        parentFlushed: !1,
        id: -1,
        index: t,
        chunks: [],
        children: [],
        preambleChildren: [],
        parentFormatContext: d,
        boundary: c,
        lastPushedText: b,
        textEmbedded: w
      };
    }
    function Zl() {
      if (r === null || r.componentStack === null)
        return "";
      var e = r.componentStack;
      try {
        var t = "";
        if (typeof e.type == "string")
          t += Tr(e.type);
        else if (typeof e.type == "function") {
          if (!e.owner) {
            var c = t, d = e.type, b = d ? d.displayName || d.name : "", w = b ? Tr(b) : "";
            t = c + w;
          }
        } else
          e.owner || (t += xu(e.type));
        for (; e; )
          c = null, e.debugStack != null ? c = Vo(
            e.debugStack
          ) : (w = e, w.stack != null && (c = typeof w.stack != "string" ? w.stack = Vo(
            w.stack
          ) : w.stack)), (e = e.owner) && c && (t += `
` + c);
        var k = t;
      } catch (A) {
        k = `
Error generating stack: ` + A.message + `
` + A.stack;
      }
      return k;
    }
    function oo(e, t) {
      if (t != null)
        for (var c = t.length - 1; 0 <= c; c--) {
          var d = t[c];
          if (typeof d.name == "string" || typeof d.time == "number") break;
          if (d.awaited != null) {
            var b = d.debugStack == null ? d.awaited : d;
            if (b.debugStack !== void 0) {
              e.componentStack = {
                parent: e.componentStack,
                type: d,
                owner: b.owner,
                stack: b.debugStack
              }, e.debugTask = b.debugTask;
              break;
            }
          }
        }
    }
    function hi(e, t) {
      if (t != null)
        for (var c = 0; c < t.length; c++) {
          var d = t[c];
          typeof d.name == "string" && d.debugStack !== void 0 && (e.componentStack = {
            parent: e.componentStack,
            type: d,
            owner: d.owner,
            stack: d.debugStack
          }, e.debugTask = d.debugTask);
        }
    }
    function gi(e) {
      var t = e.node;
      if (typeof t == "object" && t !== null)
        switch (t.$$typeof) {
          case Bc:
            var c = t.type, d = t._owner, b = t._debugStack;
            hi(e, t._debugInfo), e.debugTask = t._debugTask, e.componentStack = {
              parent: e.componentStack,
              type: c,
              owner: d,
              stack: b
            };
            break;
          case Qi:
            hi(e, t._debugInfo);
            break;
          default:
            typeof t.then == "function" && hi(e, t._debugInfo);
        }
    }
    function co(e) {
      return e === null ? null : {
        parent: e.parent,
        type: "Suspense Fallback",
        owner: e.owner,
        stack: e.stack
      };
    }
    function Ie(e) {
      var t = {};
      return e && Object.defineProperty(t, "componentStack", {
        configurable: !0,
        enumerable: !0,
        get: function() {
          try {
            var c = "", d = e;
            do
              c += xu(d.type), d = d.parent;
            while (d);
            var b = c;
          } catch (w) {
            b = `
Error generating stack: ` + w.message + `
` + w.stack;
          }
          return Object.defineProperty(t, "componentStack", {
            value: b
          }), b;
        }
      }), t;
    }
    function Pl(e, t, c, d, b) {
      e.errorDigest = t, c instanceof Error ? (t = String(c.message), c = String(c.stack)) : (t = typeof c == "object" && c !== null ? Kn(c) : String(c), c = null), b = b ? `Switched to client rendering because the server rendering aborted due to:

` : `Switched to client rendering because the server rendering errored:

`, e.errorMessage = b + t, e.errorStack = c !== null ? b + c : null, e.errorComponentStack = d.componentStack;
    }
    function it(e, t, c, d) {
      if (e = e.onError, t = d ? d.run(e.bind(null, t, c)) : e(t, c), t != null && typeof t != "string")
        console.error(
          'onError returned something with a type other than "string". onError should return a string and may return null or undefined but must not return anything else. It received something of type "%s" instead',
          typeof t
        );
      else return t;
    }
    function Jl(e, t, c, d) {
      c = e.onShellError;
      var b = e.onFatalError;
      d ? (d.run(c.bind(null, t)), d.run(b.bind(null, t))) : (c(t), b(t)), e.destination !== null ? (e.status = dr, zo(e.destination, t)) : (e.status = 13, e.fatalError = t);
    }
    function Tt(e, t) {
      Ui(e, t.next, t.hoistables);
    }
    function Ui(e, t, c) {
      for (; t !== null; ) {
        c !== null && (je(t.hoistables, c), t.inheritedHoistables = c);
        var d = t.boundaries;
        if (d !== null) {
          t.boundaries = null;
          for (var b = 0; b < d.length; b++) {
            var w = d[b];
            c !== null && je(
              w.contentState,
              c
            ), Zi(e, w, null, null);
          }
        }
        if (t.pendingTasks--, 0 < t.pendingTasks) break;
        c = t.hoistables, t = t.next;
      }
    }
    function wa(e, t) {
      var c = t.boundaries;
      if (c !== null && t.pendingTasks === c.length) {
        for (var d = !0, b = 0; b < c.length; b++) {
          var w = c[b];
          if (w.pendingTasks !== 1 || w.parentFlushed || wr(e, w)) {
            d = !1;
            break;
          }
        }
        d && Ui(e, t, t.hoistables);
      }
    }
    function Vl(e) {
      var t = {
        pendingTasks: 1,
        boundaries: null,
        hoistables: ba(),
        inheritedHoistables: null,
        together: !1,
        next: null
      };
      return e !== null && 0 < e.pendingTasks && (t.pendingTasks++, t.boundaries = [], e.next = t), t;
    }
    function vi(e, t, c, d, b) {
      var w = t.keyPath, k = t.treeContext, A = t.row, X = t.componentStack, O = t.debugTask;
      hi(t, t.node.props.children._debugInfo), t.keyPath = c, c = d.length;
      var z = null;
      if (t.replay !== null) {
        var ee = t.replay.slots;
        if (ee !== null && typeof ee == "object")
          for (var re = 0; re < c; re++) {
            var te = b !== "backwards" && b !== "unstable_legacy-backwards" ? re : c - 1 - re, j = d[te];
            t.row = z = Vl(
              z
            ), t.treeContext = xa(k, c, te);
            var we = ee[te];
            typeof we == "number" ? (pa(e, t, we, j, te), delete ee[te]) : Pt(e, t, j, te), --z.pendingTasks === 0 && Tt(e, z);
          }
        else
          for (ee = 0; ee < c; ee++)
            re = b !== "backwards" && b !== "unstable_legacy-backwards" ? ee : c - 1 - ee, te = d[re], Fl(e, t, te), t.row = z = Vl(z), t.treeContext = xa(k, c, re), Pt(e, t, te, re), --z.pendingTasks === 0 && Tt(e, z);
      } else if (b !== "backwards" && b !== "unstable_legacy-backwards")
        for (b = 0; b < c; b++)
          ee = d[b], Fl(e, t, ee), t.row = z = Vl(z), t.treeContext = xa(
            k,
            c,
            b
          ), Pt(e, t, ee, b), --z.pendingTasks === 0 && Tt(e, z);
      else {
        for (b = t.blockedSegment, ee = b.children.length, re = b.chunks.length, te = c - 1; 0 <= te; te--) {
          j = d[te], t.row = z = Vl(
            z
          ), t.treeContext = xa(k, c, te), we = di(
            e,
            re,
            null,
            t.formatContext,
            te === 0 ? b.lastPushedText : !0,
            !0
          ), b.children.splice(ee, 0, we), t.blockedSegment = we, Fl(e, t, j);
          try {
            Pt(e, t, j, te), we.lastPushedText && we.textEmbedded && we.chunks.push(ct), we.status = bn, ml(e, t.blockedBoundary, we), --z.pendingTasks === 0 && Tt(e, z);
          } catch (pn) {
            throw we.status = e.status === 12 ? Xn : Ve, pn;
          }
        }
        t.blockedSegment = b, b.lastPushedText = !1;
      }
      A !== null && z !== null && 0 < z.pendingTasks && (A.pendingTasks++, z.next = A), t.treeContext = k, t.row = A, t.keyPath = w, t.componentStack = X, t.debugTask = O;
    }
    function at(e, t, c, d, b, w) {
      var k = t.thenableState;
      for (t.thenableState = null, Lt = {}, mi = t, fr = e, ca = c, Ll = !1, vc = st = 0, iu = -1, fs = 0, mo = k, e = Cn(d, b, w); Ai; )
        Ai = !1, vc = st = 0, iu = -1, fs = 0, bc += 1, fn = null, e = d(b, w);
      return Ta(), e;
    }
    function bi(e, t, c, d, b, w, k) {
      var A = !1;
      if (w !== 0 && e.formState !== null) {
        var X = t.blockedSegment;
        if (X !== null) {
          A = !0, X = X.chunks;
          for (var O = 0; O < w; O++)
            O === k ? X.push(La) : X.push(ar);
        }
      }
      w = t.keyPath, t.keyPath = c, b ? (c = t.treeContext, t.treeContext = xa(c, 1, 0), Pt(e, t, d, -1), t.treeContext = c) : A ? Pt(e, t, d, -1) : Er(e, t, d, -1), t.keyPath = w;
    }
    function yi(e, t, c, d, b, w) {
      if (typeof d == "function")
        if (d.prototype && d.prototype.isReactComponent) {
          var k = b;
          if ("ref" in b) {
            k = {};
            for (var A in b)
              A !== "ref" && (k[A] = b[A]);
          }
          var X = d.defaultProps;
          if (X) {
            k === b && (k = Pn({}, k, b));
            for (var O in X)
              k[O] === void 0 && (k[O] = X[O]);
          }
          var z = k, ee = tl, re = d.contextType;
          if ("contextType" in d && re !== null && (re === void 0 || re.$$typeof !== On) && !xs.has(d)) {
            xs.add(d);
            var te = re === void 0 ? " However, it is set to undefined. This can be caused by a typo or by mixing up named and default imports. This can also happen due to a circular dependency, so try moving the createContext() call to a separate file." : typeof re != "object" ? " However, it is set to a " + typeof re + "." : re.$$typeof === Rr ? " Did you accidentally pass the Context.Consumer instead?" : " However, it is set to an object with keys {" + Object.keys(re).join(", ") + "}.";
            console.error(
              "%s defines an invalid contextType. contextType should point to the Context object returned by React.createContext().%s",
              Ye(d) || "Component",
              te
            );
          }
          typeof re == "object" && re !== null && (ee = re._currentValue);
          var j = new d(z, ee);
          if (typeof d.getDerivedStateFromProps == "function" && (j.state === null || j.state === void 0)) {
            var we = Ye(d) || "Component";
            tu.has(we) || (tu.add(we), console.error(
              "`%s` uses `getDerivedStateFromProps` but its initial state is %s. This is not recommended. Instead, define the initial state by assigning an object to `this.state` in the constructor of `%s`. This ensures that `getDerivedStateFromProps` arguments have a consistent shape.",
              we,
              j.state === null ? "null" : "undefined",
              we
            ));
          }
          if (typeof d.getDerivedStateFromProps == "function" || typeof j.getSnapshotBeforeUpdate == "function") {
            var pn = null, ye = null, Ue = null;
            if (typeof j.componentWillMount == "function" && j.componentWillMount.__suppressDeprecationWarning !== !0 ? pn = "componentWillMount" : typeof j.UNSAFE_componentWillMount == "function" && (pn = "UNSAFE_componentWillMount"), typeof j.componentWillReceiveProps == "function" && j.componentWillReceiveProps.__suppressDeprecationWarning !== !0 ? ye = "componentWillReceiveProps" : typeof j.UNSAFE_componentWillReceiveProps == "function" && (ye = "UNSAFE_componentWillReceiveProps"), typeof j.componentWillUpdate == "function" && j.componentWillUpdate.__suppressDeprecationWarning !== !0 ? Ue = "componentWillUpdate" : typeof j.UNSAFE_componentWillUpdate == "function" && (Ue = "UNSAFE_componentWillUpdate"), pn !== null || ye !== null || Ue !== null) {
              var jn = Ye(d) || "Component", ft = typeof d.getDerivedStateFromProps == "function" ? "getDerivedStateFromProps()" : "getSnapshotBeforeUpdate()";
              cs.has(jn) || (cs.add(
                jn
              ), console.error(
                `Unsafe legacy lifecycles will not be called for components using new component APIs.

%s uses %s but also contains the following legacy lifecycles:%s%s%s

The above lifecycles should be removed. Learn more about this warning here:
https://react.dev/link/unsafe-component-lifecycles`,
                jn,
                ft,
                pn !== null ? `
  ` + pn : "",
                ye !== null ? `
  ` + ye : "",
                Ue !== null ? `
  ` + Ue : ""
              ));
            }
          }
          var an = Ye(d) || "Component";
          j.render || (d.prototype && typeof d.prototype.render == "function" ? console.error(
            "No `render` method found on the %s instance: did you accidentally return an object from the constructor?",
            an
          ) : console.error(
            "No `render` method found on the %s instance: you may have forgotten to define `render`.",
            an
          )), !j.getInitialState || j.getInitialState.isReactClassApproved || j.state || console.error(
            "getInitialState was defined on %s, a plain JavaScript class. This is only supported for classes created using React.createClass. Did you mean to define a state property instead?",
            an
          ), j.getDefaultProps && !j.getDefaultProps.isReactClassApproved && console.error(
            "getDefaultProps was defined on %s, a plain JavaScript class. This is only supported for classes created using React.createClass. Use a static property to define defaultProps instead.",
            an
          ), j.contextType && console.error(
            "contextType was defined as an instance property on %s. Use a static property to define contextType instead.",
            an
          ), d.childContextTypes && !ru.has(d) && (ru.add(d), console.error(
            "%s uses the legacy childContextTypes API which was removed in React 19. Use React.createContext() instead. (https://react.dev/link/legacy-context)",
            an
          )), d.contextTypes && !us.has(d) && (us.add(d), console.error(
            "%s uses the legacy contextTypes API which was removed in React 19. Use React.createContext() with static contextType instead. (https://react.dev/link/legacy-context)",
            an
          )), typeof j.componentShouldUpdate == "function" && console.error(
            "%s has a method called componentShouldUpdate(). Did you mean shouldComponentUpdate()? The name is phrased as a question because the function is expected to return a value.",
            an
          ), d.prototype && d.prototype.isPureReactComponent && typeof j.shouldComponentUpdate < "u" && console.error(
            "%s has a method called shouldComponentUpdate(). shouldComponentUpdate should not be used when extending React.PureComponent. Please extend React.Component if shouldComponentUpdate is used.",
            Ye(d) || "A pure component"
          ), typeof j.componentDidUnmount == "function" && console.error(
            "%s has a method called componentDidUnmount(). But there is no such lifecycle method. Did you mean componentWillUnmount()?",
            an
          ), typeof j.componentDidReceiveProps == "function" && console.error(
            "%s has a method called componentDidReceiveProps(). But there is no such lifecycle method. If you meant to update the state in response to changing props, use componentWillReceiveProps(). If you meant to fetch data or run side-effects or mutations after React has updated the UI, use componentDidUpdate().",
            an
          ), typeof j.componentWillRecieveProps == "function" && console.error(
            "%s has a method called componentWillRecieveProps(). Did you mean componentWillReceiveProps()?",
            an
          ), typeof j.UNSAFE_componentWillRecieveProps == "function" && console.error(
            "%s has a method called UNSAFE_componentWillRecieveProps(). Did you mean UNSAFE_componentWillReceiveProps()?",
            an
          );
          var be = j.props !== z;
          j.props !== void 0 && be && console.error(
            "When calling super() in `%s`, make sure to pass up the same props that your component's constructor was passed.",
            an
          ), j.defaultProps && console.error(
            "Setting defaultProps as an instance property on %s is not supported and will be ignored. Instead, define defaultProps as a static property on %s.",
            an,
            an
          ), typeof j.getSnapshotBeforeUpdate != "function" || typeof j.componentDidUpdate == "function" || Xa.has(d) || (Xa.add(d), console.error(
            "%s: getSnapshotBeforeUpdate() should be used with componentDidUpdate(). This component defines getSnapshotBeforeUpdate() only.",
            Ye(d)
          )), typeof j.getDerivedStateFromProps == "function" && console.error(
            "%s: getDerivedStateFromProps() is defined as an instance method and will be ignored. Instead, declare it as a static method.",
            an
          ), typeof j.getDerivedStateFromError == "function" && console.error(
            "%s: getDerivedStateFromError() is defined as an instance method and will be ignored. Instead, declare it as a static method.",
            an
          ), typeof d.getSnapshotBeforeUpdate == "function" && console.error(
            "%s: getSnapshotBeforeUpdate() is defined as a static method and will be ignored. Instead, declare it as an instance method.",
            an
          );
          var dt = j.state;
          dt && (typeof dt != "object" || Kl(dt)) && console.error("%s.state: must be set to an object or null", an), typeof j.getChildContext == "function" && typeof d.childContextTypes != "object" && console.error(
            "%s.getChildContext(): childContextTypes must be defined in order to use getChildContext().",
            an
          );
          var hr = j.state !== void 0 ? j.state : null;
          j.updater = Ts, j.props = z, j.state = hr;
          var En = { queue: [], replace: !1 };
          j._reactInternals = En;
          var zn = d.contextType;
          if (j.context = typeof zn == "object" && zn !== null ? zn._currentValue : tl, j.state === z) {
            var Nr = Ye(d) || "Component";
            Lu.has(
              Nr
            ) || (Lu.add(
              Nr
            ), console.error(
              "%s: It is not recommended to assign props directly to state because updates to props won't be reflected in state. In most cases, it is better to use props directly.",
              Nr
            ));
          }
          var Kt = d.getDerivedStateFromProps;
          if (typeof Kt == "function") {
            var At = Kt(
              z,
              hr
            );
            if (At === void 0) {
              var Bn = Ye(d) || "Component";
              Po.has(Bn) || (Po.add(Bn), console.error(
                "%s.getDerivedStateFromProps(): A valid state object (or null) must be returned. You have returned undefined.",
                Bn
              ));
            }
            var pt = At == null ? hr : Pn({}, hr, At);
            j.state = pt;
          }
          if (typeof d.getDerivedStateFromProps != "function" && typeof j.getSnapshotBeforeUpdate != "function" && (typeof j.UNSAFE_componentWillMount == "function" || typeof j.componentWillMount == "function")) {
            var Wr = j.state;
            if (typeof j.componentWillMount == "function") {
              if (j.componentWillMount.__suppressDeprecationWarning !== !0) {
                var rl = Ye(d) || "Unknown";
                nu[rl] || (console.warn(
                  `componentWillMount has been renamed, and is not recommended for use. See https://react.dev/link/unsafe-component-lifecycles for details.

* Move code from componentWillMount to componentDidMount (preferred in most cases) or the constructor.

Please update the following components: %s`,
                  rl
                ), nu[rl] = !0);
              }
              j.componentWillMount();
            }
            if (typeof j.UNSAFE_componentWillMount == "function" && j.UNSAFE_componentWillMount(), Wr !== j.state && (console.error(
              "%s.componentWillMount(): Assigning directly to this.state is deprecated (except inside a component's constructor). Use setState instead.",
              Ye(d) || "Component"
            ), Ts.enqueueReplaceState(
              j,
              j.state,
              null
            )), En.queue !== null && 0 < En.queue.length) {
              var It = En.queue, Zn = En.replace;
              if (En.queue = null, En.replace = !1, Zn && It.length === 1)
                j.state = It[0];
              else {
                for (var gr = Zn ? It[0] : j.state, Hr = !0, vr = Zn ? 1 : 0; vr < It.length; vr++) {
                  var Ii = It[vr], Ur = typeof Ii == "function" ? Ii.call(
                    j,
                    gr,
                    z,
                    void 0
                  ) : Ii;
                  Ur != null && (Hr ? (Hr = !1, gr = Pn(
                    {},
                    gr,
                    Ur
                  )) : Pn(gr, Ur));
                }
                j.state = gr;
              }
            } else En.queue = null;
          }
          var ht = ln(j);
          if (e.status === 12) throw null;
          j.props !== z && (Br || console.error(
            "It looks like %s is reassigning its own `this.props` while rendering. This is not supported and can lead to confusing bugs.",
            Ye(d) || "a component"
          ), Br = !0);
          var vl = t.keyPath;
          t.keyPath = c, Er(e, t, ht, -1), t.keyPath = vl;
        } else {
          if (d.prototype && typeof d.prototype.render == "function") {
            var Bt = Ye(d) || "Unknown";
            Qt[Bt] || (console.error(
              "The <%s /> component appears to have a render method, but doesn't extend React.Component. This is likely to cause errors. Change %s to extend React.Component instead.",
              Bt,
              Bt
            ), Qt[Bt] = !0);
          }
          var Bu = at(
            e,
            t,
            c,
            d,
            b,
            void 0
          );
          if (e.status === 12) throw null;
          var ii = st !== 0, $n = vc, bl = iu;
          if (d.contextTypes) {
            var br = Ye(d) || "Unknown";
            Ln[br] || (Ln[br] = !0, console.error(
              "%s uses the legacy contextTypes API which was removed in React 19. Use React.createContext() with React.useContext() instead. (https://react.dev/link/legacy-context)",
              br
            ));
          }
          if (d && d.childContextTypes && console.error(
            `childContextTypes cannot be defined on a function component.
  %s.childContextTypes = ...`,
            d.displayName || d.name || "Component"
          ), typeof d.getDerivedStateFromProps == "function") {
            var xc = Ye(d) || "Unknown";
            Ao[xc] || (console.error(
              "%s: Function components do not support getDerivedStateFromProps.",
              xc
            ), Ao[xc] = !0);
          }
          if (typeof d.contextType == "object" && d.contextType !== null) {
            var Io = Ye(d) || "Unknown";
            zr[Io] || (console.error(
              "%s: Function components do not support contextType.",
              Io
            ), zr[Io] = !0);
          }
          bi(
            e,
            t,
            c,
            Bu,
            ii,
            $n,
            bl
          );
        }
      else if (typeof d == "string") {
        var zl = t.blockedSegment;
        if (zl === null) {
          var Mo = b.children, Yr = t.formatContext, Tc = t.keyPath;
          t.formatContext = We(Yr, d, b), t.keyPath = c, Pt(e, t, Mo, -1), t.formatContext = Yr, t.keyPath = Tc;
        } else {
          var Nu = Xu(
            zl.chunks,
            d,
            b,
            e.resumableState,
            e.renderState,
            t.blockedPreamble,
            t.hoistableState,
            t.formatContext,
            zl.lastPushedText
          );
          zl.lastPushedText = !1;
          var yl = t.formatContext, wc = t.keyPath;
          if (t.keyPath = c, (t.formatContext = We(
            yl,
            d,
            b
          )).insertionMode === ic) {
            var yr = di(
              e,
              0,
              null,
              t.formatContext,
              !1,
              !1
            );
            zl.preambleChildren.push(yr), t.blockedSegment = yr;
            try {
              yr.status = 6, Pt(e, t, Nu, -1), yr.lastPushedText && yr.textEmbedded && yr.chunks.push(ct), yr.status = bn, ml(e, t.blockedBoundary, yr);
            } finally {
              t.blockedSegment = zl;
            }
          } else Pt(e, t, Nu, -1);
          t.formatContext = yl, t.keyPath = wc;
          e: {
            var Oo = zl.chunks, Va = e.resumableState;
            switch (d) {
              case "title":
              case "style":
              case "script":
              case "area":
              case "base":
              case "br":
              case "col":
              case "embed":
              case "hr":
              case "img":
              case "input":
              case "keygen":
              case "link":
              case "meta":
              case "param":
              case "source":
              case "track":
              case "wbr":
                break e;
              case "body":
                if (yl.insertionMode <= ji) {
                  Va.hasBody = !0;
                  break e;
                }
                break;
              case "html":
                if (yl.insertionMode === bo) {
                  Va.hasHtml = !0;
                  break e;
                }
                break;
              case "head":
                if (yl.insertionMode <= ji) break e;
            }
            Oo.push(et(d));
          }
          zl.lastPushedText = !1;
        }
      } else {
        switch (d) {
          case bs:
          case nc:
          case tc:
          case Ti:
            var ua = t.keyPath;
            t.keyPath = c, Er(e, t, b.children, -1), t.keyPath = ua;
            return;
          case _t:
            var xl = t.blockedSegment;
            if (xl === null) {
              if (b.mode !== "hidden") {
                var Tl = t.keyPath;
                t.keyPath = c, Pt(e, t, b.children, -1), t.keyPath = Tl;
              }
            } else if (b.mode !== "hidden") {
              xl.chunks.push(Ou), xl.lastPushedText = !1;
              var ai = t.keyPath;
              t.keyPath = c, Pt(e, t, b.children, -1), t.keyPath = ai, xl.chunks.push(Vc), xl.lastPushedText = !1;
            }
            return;
          case ka:
            e: {
              var Gr = b.children, qt = b.revealOrder;
              if (qt === "forwards" || qt === "backwards" || qt === "unstable_legacy-backwards") {
                if (Kl(Gr)) {
                  vi(
                    e,
                    t,
                    c,
                    Gr,
                    qt
                  );
                  break e;
                }
                var sa = G(Gr);
                if (sa) {
                  var oi = sa.call(Gr);
                  if (oi) {
                    Ea(
                      t,
                      Gr,
                      -1,
                      oi,
                      sa
                    );
                    var Qa = oi.next();
                    if (!Qa.done) {
                      var Mi = [];
                      do
                        Mi.push(Qa.value), Qa = oi.next();
                      while (!Qa.done);
                      vi(
                        e,
                        t,
                        c,
                        Gr,
                        qt
                      );
                    }
                    break e;
                  }
                }
              }
              if (qt === "together") {
                var ds = t.keyPath, Fr = t.row, Bl = t.row = Vl(null);
                Bl.boundaries = [], Bl.together = !0, t.keyPath = c, Er(e, t, Gr, -1), --Bl.pendingTasks === 0 && Tt(e, Bl), t.keyPath = ds, t.row = Fr, Fr !== null && 0 < Bl.pendingTasks && (Fr.pendingTasks++, Bl.next = Fr);
              } else {
                var pc = t.keyPath;
                t.keyPath = c, Er(e, t, Gr, -1), t.keyPath = pc;
              }
            }
            return;
          case $u:
          case ku:
            throw Error(
              "ReactDOMServer does not yet support scope components."
            );
          case Vi:
            e: if (t.replay !== null) {
              var au = t.keyPath, ou = t.formatContext, zs = t.row;
              t.keyPath = c, t.formatContext = Ct(
                e.resumableState,
                ou
              ), t.row = null;
              var Ka = b.children;
              try {
                Pt(e, t, Ka, -1);
              } finally {
                t.keyPath = au, t.formatContext = ou, t.row = zs;
              }
            } else {
              var _o = t.keyPath, qa = t.formatContext, cu = t.row, ms = t.blockedBoundary, ws = t.blockedPreamble, mr = t.hoistableState, ci = t.blockedSegment, ja = b.fallback, Wu = b.children, wl = /* @__PURE__ */ new Set(), gt = t.formatContext.insertionMode < rr ? fi(
                e,
                t.row,
                wl,
                le(),
                le()
              ) : fi(
                e,
                t.row,
                wl,
                null,
                null
              );
              e.trackedPostpones !== null && (gt.trackedContentKeyPath = c);
              var Jn = di(
                e,
                ci.chunks.length,
                gt,
                t.formatContext,
                !1,
                !1
              );
              ci.children.push(Jn), ci.lastPushedText = !1;
              var mn = di(
                e,
                0,
                null,
                t.formatContext,
                !1,
                !1
              );
              if (mn.parentFlushed = !0, e.trackedPostpones !== null) {
                var ps = t.componentStack, Do = [
                  c[0],
                  "Suspense Fallback",
                  c[2]
                ], Hu = [
                  Do[1],
                  Do[2],
                  [],
                  null
                ];
                e.trackedPostpones.workingMap.set(
                  Do,
                  Hu
                ), gt.trackedFallbackNode = Hu, t.blockedSegment = Jn, t.blockedPreamble = gt.fallbackPreamble, t.keyPath = Do, t.formatContext = He(
                  e.resumableState,
                  qa
                ), t.componentStack = co(
                  ps
                ), Jn.status = 6;
                try {
                  Pt(e, t, ja, -1), Jn.lastPushedText && Jn.textEmbedded && Jn.chunks.push(ct), Jn.status = bn, ml(e, ms, Jn);
                } catch (Ns) {
                  throw Jn.status = e.status === 12 ? Xn : Ve, Ns;
                } finally {
                  t.blockedSegment = ci, t.blockedPreamble = ws, t.keyPath = _o, t.formatContext = qa;
                }
                var Lo = Sl(
                  e,
                  null,
                  Wu,
                  -1,
                  gt,
                  mn,
                  gt.contentPreamble,
                  gt.contentState,
                  t.abortSet,
                  c,
                  Ct(
                    e.resumableState,
                    t.formatContext
                  ),
                  t.context,
                  t.treeContext,
                  null,
                  ps,
                  tl,
                  t.debugTask
                );
                gi(Lo), e.pingedTasks.push(Lo);
              } else {
                t.blockedBoundary = gt, t.blockedPreamble = gt.contentPreamble, t.hoistableState = gt.contentState, t.blockedSegment = mn, t.keyPath = c, t.formatContext = Ct(
                  e.resumableState,
                  qa
                ), t.row = null, mn.status = 6;
                try {
                  if (Pt(e, t, Wu, -1), mn.lastPushedText && mn.textEmbedded && mn.chunks.push(ct), mn.status = bn, ml(e, gt, mn), ho(gt, mn), gt.pendingTasks === 0 && gt.status === vn) {
                    if (gt.status = bn, !wr(e, gt)) {
                      cu !== null && --cu.pendingTasks === 0 && Tt(e, cu), e.pendingRootTasks === 0 && t.blockedPreamble && Ca(e);
                      break e;
                    }
                  } else
                    cu !== null && cu.together && wa(e, cu);
                } catch (Ns) {
                  if (gt.status = he, e.status === 12) {
                    mn.status = Xn;
                    var Ec = e.fatalError;
                  } else
                    mn.status = Ve, Ec = Ns;
                  var Uu = Ie(t.componentStack), Rc = it(
                    e,
                    Ec,
                    Uu,
                    t.debugTask
                  );
                  Pl(
                    gt,
                    Rc,
                    Ec,
                    Uu,
                    !1
                  ), uo(e, gt);
                } finally {
                  t.blockedBoundary = ms, t.blockedPreamble = ws, t.hoistableState = mr, t.blockedSegment = ci, t.keyPath = _o, t.formatContext = qa, t.row = cu;
                }
                var uu = Sl(
                  e,
                  null,
                  ja,
                  -1,
                  ms,
                  Jn,
                  gt.fallbackPreamble,
                  gt.fallbackState,
                  wl,
                  [c[0], "Suspense Fallback", c[2]],
                  He(
                    e.resumableState,
                    t.formatContext
                  ),
                  t.context,
                  t.treeContext,
                  t.row,
                  co(
                    t.componentStack
                  ),
                  tl,
                  t.debugTask
                );
                gi(uu), e.pingedTasks.push(uu);
              }
            }
            return;
        }
        if (typeof d == "object" && d !== null)
          switch (d.$$typeof) {
            case en:
              if ("ref" in b) {
                var su = {};
                for (var hs in b)
                  hs !== "ref" && (su[hs] = b[hs]);
              } else su = b;
              var Es = at(
                e,
                t,
                c,
                d.render,
                su,
                w
              );
              bi(
                e,
                t,
                c,
                Es,
                st !== 0,
                vc,
                iu
              );
              return;
            case Sa:
              yi(e, t, c, d.type, b, w);
              return;
            case On:
              var As = b.value, Yu = b.children, Gu = t.context, Cc = t.keyPath, $a = d._currentValue;
              d._currentValue = As, d._currentRenderer !== void 0 && d._currentRenderer !== null && d._currentRenderer !== oa && console.error(
                "Detected multiple renderers concurrently rendering the same context provider. This is currently unsupported."
              ), d._currentRenderer = oa;
              var fa = Fi, Xr = {
                parent: fa,
                depth: fa === null ? 0 : fa.depth + 1,
                context: d,
                parentValue: $a,
                value: As
              };
              Fi = Xr, t.context = Xr, t.keyPath = c, Er(e, t, Yu, -1);
              var fu = Fi;
              if (fu === null)
                throw Error(
                  "Tried to pop a Context at the root of the app. This is a bug in React."
                );
              fu.context !== d && console.error(
                "The parent context is not the expected context. This is probably a bug in React."
              ), fu.context._currentValue = fu.parentValue, d._currentRenderer !== void 0 && d._currentRenderer !== null && d._currentRenderer !== oa && console.error(
                "Detected multiple renderers concurrently rendering the same context provider. This is currently unsupported."
              ), d._currentRenderer = oa;
              var Is = Fi = fu.parent;
              t.context = Is, t.keyPath = Cc, Gu !== t.context && console.error(
                "Popping the context provider did not return back to the original snapshot. This is a bug in React."
              );
              return;
            case Rr:
              var Bs = d._context, Rs = b.children;
              typeof Rs != "function" && console.error(
                "A context consumer was rendered with multiple children, or a child that isn't a function. A context consumer expects a single child that is a function. If you did pass a function, make sure there is no trailing or leading whitespace around it."
              );
              var js = Rs(Bs._currentValue), gs = t.keyPath;
              t.keyPath = c, Er(e, t, js, -1), t.keyPath = gs;
              return;
            case Qi:
              var $s = Vt(d);
              if (e.status === 12) throw null;
              yi(e, t, c, $s, b, w);
              return;
          }
        var Ms = "";
        throw (d === void 0 || typeof d == "object" && d !== null && Object.keys(d).length === 0) && (Ms += " You likely forgot to export your component from the file it's defined in, or you might have mixed up default and named imports."), Error(
          "Element type is invalid: expected a string (for built-in components) or a class/function (for composite components) but got: " + ((d == null ? d : typeof d) + "." + Ms)
        );
      }
    }
    function pa(e, t, c, d, b) {
      var w = t.replay, k = t.blockedBoundary, A = di(
        e,
        0,
        null,
        t.formatContext,
        !1,
        !1
      );
      A.id = c, A.parentFlushed = !0;
      try {
        t.replay = null, t.blockedSegment = A, Pt(e, t, d, b), A.status = bn, ml(e, k, A), k === null ? e.completedRootSegment = A : (ho(k, A), k.parentFlushed && e.partialBoundaries.push(k));
      } finally {
        t.replay = w, t.blockedSegment = null;
      }
    }
    function Yi(e, t, c, d, b, w, k, A, X, O) {
      w = O.nodes;
      for (var z = 0; z < w.length; z++) {
        var ee = w[z];
        if (b === ee[1]) {
          if (ee.length === 4) {
            if (d !== null && d !== ee[0])
              throw Error(
                "Expected the resume to render <" + ee[0] + "> in this slot but instead it rendered <" + d + ">. The tree doesn't match so React will fallback to client rendering."
              );
            var re = ee[2];
            d = ee[3], b = t.node, t.replay = { nodes: re, slots: d, pendingTasks: 1 };
            try {
              if (yi(e, t, c, k, A, X), t.replay.pendingTasks === 1 && 0 < t.replay.nodes.length)
                throw Error(
                  "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                );
              t.replay.pendingTasks--;
            } catch (be) {
              if (typeof be == "object" && be !== null && (be === ei || typeof be.then == "function"))
                throw t.node === b ? t.replay = O : w.splice(z, 1), be;
              t.replay.pendingTasks--, k = Ie(t.componentStack), A = e, e = t.blockedBoundary, c = be, X = d, d = it(A, c, k, t.debugTask), Gi(
                A,
                e,
                re,
                X,
                c,
                d,
                k,
                !1
              );
            }
            t.replay = O;
          } else {
            if (k !== Vi)
              throw Error(
                "Expected the resume to render <Suspense> in this slot but instead it rendered <" + (Ye(k) || "Unknown") + ">. The tree doesn't match so React will fallback to client rendering."
              );
            e: {
              O = void 0, d = ee[5], k = ee[2], X = ee[3], b = ee[4] === null ? [] : ee[4][2], ee = ee[4] === null ? null : ee[4][3];
              var te = t.keyPath, j = t.formatContext, we = t.row, pn = t.replay, ye = t.blockedBoundary, Ue = t.hoistableState, jn = A.children, ft = A.fallback, an = /* @__PURE__ */ new Set();
              A = t.formatContext.insertionMode < rr ? fi(
                e,
                t.row,
                an,
                le(),
                le()
              ) : fi(
                e,
                t.row,
                an,
                null,
                null
              ), A.parentFlushed = !0, A.rootSegmentID = d, t.blockedBoundary = A, t.hoistableState = A.contentState, t.keyPath = c, t.formatContext = Ct(
                e.resumableState,
                j
              ), t.row = null, t.replay = { nodes: k, slots: X, pendingTasks: 1 };
              try {
                if (Pt(e, t, jn, -1), t.replay.pendingTasks === 1 && 0 < t.replay.nodes.length)
                  throw Error(
                    "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                  );
                if (t.replay.pendingTasks--, A.pendingTasks === 0 && A.status === vn) {
                  A.status = bn, e.completedBoundaries.push(A);
                  break e;
                }
              } catch (be) {
                A.status = he, re = Ie(t.componentStack), O = it(
                  e,
                  be,
                  re,
                  t.debugTask
                ), Pl(A, O, be, re, !1), t.replay.pendingTasks--, e.clientRenderedBoundaries.push(A);
              } finally {
                t.blockedBoundary = ye, t.hoistableState = Ue, t.replay = pn, t.keyPath = te, t.formatContext = j, t.row = we;
              }
              A = wu(
                e,
                null,
                { nodes: b, slots: ee, pendingTasks: 0 },
                ft,
                -1,
                ye,
                A.fallbackState,
                an,
                [c[0], "Suspense Fallback", c[2]],
                He(
                  e.resumableState,
                  t.formatContext
                ),
                t.context,
                t.treeContext,
                t.row,
                co(
                  t.componentStack
                ),
                tl,
                t.debugTask
              ), gi(A), e.pingedTasks.push(A);
            }
          }
          w.splice(z, 1);
          break;
        }
      }
    }
    function Ea(e, t, c, d, b) {
      d === t ? (c !== -1 || e.componentStack === null || typeof e.componentStack.type != "function" || Object.prototype.toString.call(e.componentStack.type) !== "[object GeneratorFunction]" || Object.prototype.toString.call(d) !== "[object Generator]") && (ti || console.error(
        "Using Iterators as children is unsupported and will likely yield unexpected results because enumerating a generator mutates it. You may convert it to an array with `Array.from()` or the `[...spread]` operator before rendering. You can also use an Iterable that can iterate multiple times over the same items."
      ), ti = !0) : t.entries !== b || ri || (console.error(
        "Using Maps as children is not supported. Use an array of keyed ReactElements instead."
      ), ri = !0);
    }
    function Er(e, t, c, d) {
      t.replay !== null && typeof t.replay.slots == "number" ? pa(e, t, t.replay.slots, c, d) : (t.node = c, t.childIndex = d, c = t.componentStack, d = t.debugTask, gi(t), Ra(e, t), t.componentStack = c, t.debugTask = d);
    }
    function Ra(e, t) {
      var c = t.node, d = t.childIndex;
      if (c !== null) {
        if (typeof c == "object") {
          switch (c.$$typeof) {
            case Bc:
              var b = c.type, w = c.key;
              c = c.props;
              var k = c.ref;
              k = k !== void 0 ? k : null;
              var A = t.debugTask, X = Ye(b);
              w = w ?? (d === -1 ? 0 : d);
              var O = [t.keyPath, X, w];
              t.replay !== null ? A ? A.run(
                Yi.bind(
                  null,
                  e,
                  t,
                  O,
                  X,
                  w,
                  d,
                  b,
                  c,
                  k,
                  t.replay
                )
              ) : Yi(
                e,
                t,
                O,
                X,
                w,
                d,
                b,
                c,
                k,
                t.replay
              ) : A ? A.run(
                yi.bind(
                  null,
                  e,
                  t,
                  O,
                  b,
                  c,
                  k
                )
              ) : yi(e, t, O, b, c, k);
              return;
            case ec:
              throw Error(
                "Portals are not currently supported by the server renderer. Render them conditionally so that they only appear on the client render."
              );
            case Qi:
              if (b = Vt(c), e.status === 12) throw null;
              Er(e, t, b, d);
              return;
          }
          if (Kl(c)) {
            pu(e, t, c, d);
            return;
          }
          if ((w = G(c)) && (b = w.call(c))) {
            if (Ea(t, c, d, b, w), c = b.next(), !c.done) {
              w = [];
              do
                w.push(c.value), c = b.next();
              while (!c.done);
              pu(e, t, w, d);
            }
            return;
          }
          if (typeof c.then == "function")
            return t.thenableState = null, Er(
              e,
              t,
              xt(c),
              d
            );
          if (c.$$typeof === On)
            return Er(
              e,
              t,
              c._currentValue,
              d
            );
          throw e = Object.prototype.toString.call(c), Error(
            "Objects are not valid as a React child (found: " + (e === "[object Object]" ? "object with keys {" + Object.keys(c).join(", ") + "}" : e) + "). If you meant to render a collection of children, use an array instead."
          );
        }
        typeof c == "string" ? (t = t.blockedSegment, t !== null && (t.lastPushedText = _e(
          t.chunks,
          c,
          e.renderState,
          t.lastPushedText
        ))) : typeof c == "number" || typeof c == "bigint" ? (t = t.blockedSegment, t !== null && (t.lastPushedText = _e(
          t.chunks,
          "" + c,
          e.renderState,
          t.lastPushedText
        ))) : (typeof c == "function" && (e = c.displayName || c.name || "Component", console.error(
          "Functions are not valid as a React child. This may happen if you return %s instead of <%s /> from render. Or maybe you meant to call this function rather than return it.",
          e,
          e
        )), typeof c == "symbol" && console.error(
          `Symbols are not valid as a React child.
  %s`,
          String(c)
        ));
      }
    }
    function Fl(e, t, c) {
      if (c !== null && typeof c == "object" && (c.$$typeof === Bc || c.$$typeof === ec) && c._store && (!c._store.validated && c.key == null || c._store.validated === 2)) {
        if (typeof c._store != "object")
          throw Error(
            "React Component in warnForMissingKey should have a _store. This error is likely caused by a bug in React. Please file an issue."
          );
        c._store.validated = 1;
        var d = e.didWarnForKey;
        if (d == null && (d = e.didWarnForKey = /* @__PURE__ */ new WeakSet()), e = t.componentStack, e !== null && !d.has(e)) {
          d.add(e);
          var b = Ye(c.type);
          d = c._owner;
          var w = e.owner;
          if (e = "", w && typeof w.type < "u") {
            var k = Ye(w.type);
            k && (e = `

Check the render method of \`` + k + "`.");
          }
          e || b && (e = `

Check the top-level render call using <` + b + ">."), b = "", d != null && w !== d && (w = null, typeof d.type < "u" ? w = Ye(d.type) : typeof d.name == "string" && (w = d.name), w && (b = " It was passed a child from " + w + ".")), d = t.componentStack, t.componentStack = {
            parent: t.componentStack,
            type: c.type,
            owner: c._owner,
            stack: c._debugStack
          }, console.error(
            'Each child in a list should have a unique "key" prop.%s%s See https://react.dev/link/warning-keys for more information.',
            e,
            b
          ), t.componentStack = d;
        }
      }
    }
    function pu(e, t, c, d) {
      var b = t.keyPath, w = t.componentStack, k = t.debugTask;
      if (hi(t, t.node._debugInfo), d !== -1 && (t.keyPath = [t.keyPath, "Fragment", d], t.replay !== null)) {
        for (var A = t.replay, X = A.nodes, O = 0; O < X.length; O++) {
          var z = X[O];
          if (z[1] === d) {
            d = z[2], z = z[3], t.replay = { nodes: d, slots: z, pendingTasks: 1 };
            try {
              if (pu(e, t, c, -1), t.replay.pendingTasks === 1 && 0 < t.replay.nodes.length)
                throw Error(
                  "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                );
              t.replay.pendingTasks--;
            } catch (j) {
              if (typeof j == "object" && j !== null && (j === ei || typeof j.then == "function"))
                throw j;
              t.replay.pendingTasks--;
              var ee = Ie(t.componentStack);
              c = t.blockedBoundary;
              var re = j, te = z;
              z = it(
                e,
                re,
                ee,
                t.debugTask
              ), Gi(
                e,
                c,
                d,
                te,
                re,
                z,
                ee,
                !1
              );
            }
            t.replay = A, X.splice(O, 1);
            break;
          }
        }
        t.keyPath = b, t.componentStack = w, t.debugTask = k;
        return;
      }
      if (A = t.treeContext, X = c.length, t.replay !== null && (O = t.replay.slots, O !== null && typeof O == "object")) {
        for (d = 0; d < X; d++)
          z = c[d], t.treeContext = xa(
            A,
            X,
            d
          ), re = O[d], typeof re == "number" ? (pa(e, t, re, z, d), delete O[d]) : Pt(e, t, z, d);
        t.treeContext = A, t.keyPath = b, t.componentStack = w, t.debugTask = k;
        return;
      }
      for (O = 0; O < X; O++)
        d = c[O], Fl(e, t, d), t.treeContext = xa(A, X, O), Pt(e, t, d, O);
      t.treeContext = A, t.keyPath = b, t.componentStack = w, t.debugTask = k;
    }
    function Eu(e, t, c) {
      if (c.status = Oe, c.rootSegmentID = e.nextSegmentId++, e = c.trackedContentKeyPath, e === null)
        throw Error(
          "It should not be possible to postpone at the root. This is a bug in React."
        );
      var d = c.trackedFallbackNode, b = [], w = t.workingMap.get(e);
      return w === void 0 ? (c = [
        e[1],
        e[2],
        b,
        null,
        d,
        c.rootSegmentID
      ], t.workingMap.set(e, c), jr(c, e[0], t), c) : (w[4] = d, w[5] = c.rootSegmentID, w);
    }
    function Ru(e, t, c, d) {
      d.status = Oe;
      var b = c.keyPath, w = c.blockedBoundary;
      if (w === null)
        d.id = e.nextSegmentId++, t.rootSlots = d.id, e.completedRootSegment !== null && (e.completedRootSegment.status = Oe);
      else {
        if (w !== null && w.status === vn) {
          var k = Eu(
            e,
            t,
            w
          );
          if (w.trackedContentKeyPath === b && c.childIndex === -1) {
            d.id === -1 && (d.id = d.parentFlushed ? w.rootSegmentID : e.nextSegmentId++), k[3] = d.id;
            return;
          }
        }
        if (d.id === -1 && (d.id = d.parentFlushed && w !== null ? w.rootSegmentID : e.nextSegmentId++), c.childIndex === -1)
          b === null ? t.rootSlots = d.id : (c = t.workingMap.get(b), c === void 0 ? (c = [b[1], b[2], [], d.id], jr(c, b[0], t)) : c[3] = d.id);
        else {
          if (b === null) {
            if (e = t.rootSlots, e === null)
              e = t.rootSlots = {};
            else if (typeof e == "number")
              throw Error(
                "It should not be possible to postpone both at the root of an element as well as a slot below. This is a bug in React."
              );
          } else if (w = t.workingMap, k = w.get(b), k === void 0)
            e = {}, k = [b[1], b[2], [], e], w.set(b, k), jr(k, b[0], t);
          else if (e = k[3], e === null)
            e = k[3] = {};
          else if (typeof e == "number")
            throw Error(
              "It should not be possible to postpone both at the root of an element as well as a slot below. This is a bug in React."
            );
          e[c.childIndex] = d.id;
        }
      }
    }
    function uo(e, t) {
      e = e.trackedPostpones, e !== null && (t = t.trackedContentKeyPath, t !== null && (t = e.workingMap.get(t), t !== void 0 && (t.length = 4, t[2] = [], t[3] = null)));
    }
    function so(e, t, c) {
      return wu(
        e,
        c,
        t.replay,
        t.node,
        t.childIndex,
        t.blockedBoundary,
        t.hoistableState,
        t.abortSet,
        t.keyPath,
        t.formatContext,
        t.context,
        t.treeContext,
        t.row,
        t.componentStack,
        tl,
        t.debugTask
      );
    }
    function fo(e, t, c) {
      var d = t.blockedSegment, b = di(
        e,
        d.chunks.length,
        null,
        t.formatContext,
        d.lastPushedText,
        !0
      );
      return d.children.push(b), d.lastPushedText = !1, Sl(
        e,
        c,
        t.node,
        t.childIndex,
        t.blockedBoundary,
        b,
        t.blockedPreamble,
        t.hoistableState,
        t.abortSet,
        t.keyPath,
        t.formatContext,
        t.context,
        t.treeContext,
        t.row,
        t.componentStack,
        tl,
        t.debugTask
      );
    }
    function Pt(e, t, c, d) {
      var b = t.formatContext, w = t.context, k = t.keyPath, A = t.treeContext, X = t.componentStack, O = t.debugTask, z = t.blockedSegment;
      if (z === null) {
        z = t.replay;
        try {
          return Er(e, t, c, d);
        } catch (te) {
          if (Ta(), c = te === ei ? yu() : te, e.status !== 12 && typeof c == "object" && c !== null) {
            if (typeof c.then == "function") {
              d = te === ei ? Vr() : null, e = so(
                e,
                t,
                d
              ).ping, c.then(e, e), t.formatContext = b, t.context = w, t.keyPath = k, t.treeContext = A, t.componentStack = X, t.replay = z, t.debugTask = O, $e(w);
              return;
            }
            if (c.message === "Maximum call stack size exceeded") {
              c = te === ei ? Vr() : null, c = so(e, t, c), e.pingedTasks.push(c), t.formatContext = b, t.context = w, t.keyPath = k, t.treeContext = A, t.componentStack = X, t.replay = z, t.debugTask = O, $e(w);
              return;
            }
          }
        }
      } else {
        var ee = z.children.length, re = z.chunks.length;
        try {
          return Er(e, t, c, d);
        } catch (te) {
          if (Ta(), z.children.length = ee, z.chunks.length = re, c = te === ei ? yu() : te, e.status !== 12 && typeof c == "object" && c !== null) {
            if (typeof c.then == "function") {
              z = c, c = te === ei ? Vr() : null, e = fo(e, t, c).ping, z.then(e, e), t.formatContext = b, t.context = w, t.keyPath = k, t.treeContext = A, t.componentStack = X, t.debugTask = O, $e(w);
              return;
            }
            if (c.message === "Maximum call stack size exceeded") {
              z = te === ei ? Vr() : null, z = fo(e, t, z), e.pingedTasks.push(z), t.formatContext = b, t.context = w, t.keyPath = k, t.treeContext = A, t.componentStack = X, t.debugTask = O, $e(w);
              return;
            }
          }
        }
      }
      throw t.formatContext = b, t.context = w, t.keyPath = k, t.treeContext = A, $e(w), c;
    }
    function ju(e) {
      var t = e.blockedBoundary, c = e.blockedSegment;
      c !== null && (c.status = Xn, Zi(this, t, e.row, c));
    }
    function Gi(e, t, c, d, b, w, k, A) {
      for (var X = 0; X < c.length; X++) {
        var O = c[X];
        if (O.length === 4)
          Gi(
            e,
            t,
            O[2],
            O[3],
            b,
            w,
            k,
            A
          );
        else {
          var z = e;
          O = O[5];
          var ee = b, re = w, te = k, j = A, we = fi(
            z,
            null,
            /* @__PURE__ */ new Set(),
            null,
            null
          );
          we.parentFlushed = !0, we.rootSegmentID = O, we.status = he, Pl(
            we,
            re,
            ee,
            te,
            j
          ), we.parentFlushed && z.clientRenderedBoundaries.push(we);
        }
      }
      if (c.length = 0, d !== null) {
        if (t === null)
          throw Error(
            "We should not have any resumable nodes in the shell. This is a bug in React."
          );
        if (t.status !== he && (t.status = he, Pl(
          t,
          w,
          b,
          k,
          A
        ), t.parentFlushed && e.clientRenderedBoundaries.push(t)), typeof d == "object")
          for (var pn in d) delete d[pn];
      }
    }
    function qo(e, t, c) {
      var d = e.blockedBoundary, b = e.blockedSegment;
      if (b !== null) {
        if (b.status === 6) return;
        b.status = Xn;
      }
      var w = Ie(e.componentStack), k = e.node;
      if (k !== null && typeof k == "object" && oo(e, k._debugInfo), d === null) {
        if (t.status !== 13 && t.status !== dr) {
          if (d = e.replay, d === null) {
            t.trackedPostpones !== null && b !== null ? (d = t.trackedPostpones, it(t, c, w, e.debugTask), Ru(t, d, e, b), Zi(t, null, e.row, b)) : (it(t, c, w, e.debugTask), Jl(t, c, w, e.debugTask));
            return;
          }
          d.pendingTasks--, d.pendingTasks === 0 && 0 < d.nodes.length && (b = it(t, c, w, null), Gi(
            t,
            null,
            d.nodes,
            d.slots,
            c,
            b,
            w,
            !0
          )), t.pendingRootTasks--, t.pendingRootTasks === 0 && xi(t);
        }
      } else {
        if (k = t.trackedPostpones, d.status !== he) {
          if (k !== null && b !== null)
            return it(t, c, w, e.debugTask), Ru(t, k, e, b), d.fallbackAbortableTasks.forEach(function(A) {
              return qo(A, t, c);
            }), d.fallbackAbortableTasks.clear(), Zi(t, d, e.row, b);
          d.status = he, b = it(
            t,
            c,
            w,
            e.debugTask
          ), d.status = he, Pl(d, b, c, w, !0), uo(t, d), d.parentFlushed && t.clientRenderedBoundaries.push(d);
        }
        d.pendingTasks--, w = d.row, w !== null && --w.pendingTasks === 0 && Tt(t, w), d.fallbackAbortableTasks.forEach(function(A) {
          return qo(A, t, c);
        }), d.fallbackAbortableTasks.clear();
      }
      e = e.row, e !== null && --e.pendingTasks === 0 && Tt(t, e), t.allPendingTasks--, t.allPendingTasks === 0 && Xi(t);
    }
    function _c(e, t) {
      try {
        var c = e.renderState, d = c.onHeaders;
        if (d) {
          var b = c.headers;
          if (b) {
            c.headers = null;
            var w = b.preconnects;
            if (b.fontPreloads && (w && (w += ", "), w += b.fontPreloads), b.highImagePreloads && (w && (w += ", "), w += b.highImagePreloads), !t) {
              var k = c.styles.values(), A = k.next();
              e: for (; 0 < b.remainingCapacity && !A.done; A = k.next())
                for (var X = A.value.sheets.values(), O = X.next(); 0 < b.remainingCapacity && !O.done; O = X.next()) {
                  var z = O.value, ee = z.props, re = ee.href, te = z.props, j = Wi(
                    te.href,
                    "style",
                    {
                      crossOrigin: te.crossOrigin,
                      integrity: te.integrity,
                      nonce: te.nonce,
                      type: te.type,
                      fetchPriority: te.fetchPriority,
                      referrerPolicy: te.referrerPolicy,
                      media: te.media
                    }
                  );
                  if (0 <= (b.remainingCapacity -= j.length + 2))
                    c.resets.style[re] = Ft, w && (w += ", "), w += j, c.resets.style[re] = typeof ee.crossOrigin == "string" || typeof ee.integrity == "string" ? [ee.crossOrigin, ee.integrity] : Ft;
                  else break e;
                }
            }
            d(w ? { Link: w } : {});
          }
        }
      } catch (we) {
        it(e, we, {}, null);
      }
    }
    function xi(e) {
      e.trackedPostpones === null && _c(e, !0), e.trackedPostpones === null && Ca(e), e.onShellError = er, e = e.onShellReady, e();
    }
    function Xi(e) {
      _c(
        e,
        e.trackedPostpones === null ? !0 : e.completedRootSegment === null || e.completedRootSegment.status !== Oe
      ), Ca(e), e = e.onAllReady, e();
    }
    function ho(e, t) {
      if (t.chunks.length === 0 && t.children.length === 1 && t.children[0].boundary === null && t.children[0].id === -1) {
        var c = t.children[0];
        c.id = t.id, c.parentFlushed = !0, c.status !== bn && c.status !== Xn && c.status !== Ve || ho(e, c);
      } else e.completedSegments.push(t);
    }
    function ml(e, t, c) {
      if (Zr !== null) {
        c = c.chunks;
        for (var d = 0, b = 0; b < c.length; b++)
          d += c[b].byteLength;
        t === null ? e.byteSize += d : t.byteSize += d;
      }
    }
    function Zi(e, t, c, d) {
      if (c !== null && (--c.pendingTasks === 0 ? Tt(e, c) : c.together && wa(e, c)), e.allPendingTasks--, t === null) {
        if (d !== null && d.parentFlushed) {
          if (e.completedRootSegment !== null)
            throw Error(
              "There can only be one root segment. This is a bug in React."
            );
          e.completedRootSegment = d;
        }
        e.pendingRootTasks--, e.pendingRootTasks === 0 && xi(e);
      } else if (t.pendingTasks--, t.status !== he)
        if (t.pendingTasks === 0) {
          if (t.status === vn && (t.status = bn), d !== null && d.parentFlushed && (d.status === bn || d.status === Xn) && ho(t, d), t.parentFlushed && e.completedBoundaries.push(t), t.status === bn)
            c = t.row, c !== null && je(c.hoistables, t.contentState), wr(e, t) || (t.fallbackAbortableTasks.forEach(
              ju,
              e
            ), t.fallbackAbortableTasks.clear(), c !== null && --c.pendingTasks === 0 && Tt(e, c)), e.pendingRootTasks === 0 && e.trackedPostpones === null && t.contentPreamble !== null && Ca(e);
          else if (t.status === Oe && (t = t.row, t !== null)) {
            if (e.trackedPostpones !== null) {
              c = e.trackedPostpones;
              var b = t.next;
              if (b !== null && (d = b.boundaries, d !== null))
                for (b.boundaries = null, b = 0; b < d.length; b++) {
                  var w = d[b];
                  Eu(e, c, w), Zi(e, w, null, null);
                }
            }
            --t.pendingTasks === 0 && Tt(e, t);
          }
        } else
          d === null || !d.parentFlushed || d.status !== bn && d.status !== Xn || (ho(t, d), t.completedSegments.length === 1 && t.parentFlushed && e.partialBoundaries.push(t)), t = t.row, t !== null && t.together && wa(e, t);
      e.allPendingTasks === 0 && Xi(e);
    }
    function Ji(e) {
      if (e.status !== dr && e.status !== 13) {
        var t = Fi, c = Le.H;
        Le.H = yc;
        var d = Le.A;
        Le.A = u;
        var b = yn;
        yn = e;
        var w = Le.getCurrentStack;
        Le.getCurrentStack = Zl;
        var k = n;
        n = e.resumableState;
        try {
          var A = e.pingedTasks, X;
          for (X = 0; X < A.length; X++) {
            var O = e, z = A[X], ee = z.blockedSegment;
            if (ee === null) {
              var re = void 0, te = O;
              if (O = z, O.replay.pendingTasks !== 0) {
                $e(O.context), re = r, r = O;
                try {
                  if (typeof O.replay.slots == "number" ? pa(
                    te,
                    O,
                    O.replay.slots,
                    O.node,
                    O.childIndex
                  ) : Ra(te, O), O.replay.pendingTasks === 1 && 0 < O.replay.nodes.length)
                    throw Error(
                      "Couldn't find all resumable slots by key/index during replaying. The tree doesn't match so React will fallback to client rendering."
                    );
                  O.replay.pendingTasks--, O.abortSet.delete(O), Zi(
                    te,
                    O.blockedBoundary,
                    O.row,
                    null
                  );
                } catch (Zn) {
                  Ta();
                  var j = Zn === ei ? yu() : Zn;
                  if (typeof j == "object" && j !== null && typeof j.then == "function") {
                    var we = O.ping;
                    j.then(we, we), O.thenableState = Zn === ei ? Vr() : null;
                  } else {
                    O.replay.pendingTasks--, O.abortSet.delete(O);
                    var pn = Ie(O.componentStack), ye = void 0, Ue = te, jn = O.blockedBoundary, ft = te.status === 12 ? te.fatalError : j, an = pn, be = O.replay.nodes, dt = O.replay.slots;
                    ye = it(
                      Ue,
                      ft,
                      an,
                      O.debugTask
                    ), Gi(
                      Ue,
                      jn,
                      be,
                      dt,
                      ft,
                      ye,
                      an,
                      !1
                    ), te.pendingRootTasks--, te.pendingRootTasks === 0 && xi(te), te.allPendingTasks--, te.allPendingTasks === 0 && Xi(te);
                  }
                } finally {
                  r = re;
                }
              }
            } else if (te = re = void 0, ye = z, Ue = ee, Ue.status === vn) {
              Ue.status = 6, $e(ye.context), te = r, r = ye;
              var hr = Ue.children.length, En = Ue.chunks.length;
              try {
                Ra(O, ye), Ue.lastPushedText && Ue.textEmbedded && Ue.chunks.push(ct), ye.abortSet.delete(ye), Ue.status = bn, ml(
                  O,
                  ye.blockedBoundary,
                  Ue
                ), Zi(
                  O,
                  ye.blockedBoundary,
                  ye.row,
                  Ue
                );
              } catch (Zn) {
                Ta(), Ue.children.length = hr, Ue.chunks.length = En;
                var zn = Zn === ei ? yu() : O.status === 12 ? O.fatalError : Zn;
                if (O.status === 12 && O.trackedPostpones !== null) {
                  var Nr = O.trackedPostpones, Kt = Ie(ye.componentStack);
                  ye.abortSet.delete(ye), it(
                    O,
                    zn,
                    Kt,
                    ye.debugTask
                  ), Ru(
                    O,
                    Nr,
                    ye,
                    Ue
                  ), Zi(
                    O,
                    ye.blockedBoundary,
                    ye.row,
                    Ue
                  );
                } else if (typeof zn == "object" && zn !== null && typeof zn.then == "function") {
                  Ue.status = vn, ye.thenableState = Zn === ei ? Vr() : null;
                  var At = ye.ping;
                  zn.then(At, At);
                } else {
                  var Bn = Ie(
                    ye.componentStack
                  );
                  ye.abortSet.delete(ye), Ue.status = Ve;
                  var pt = ye.blockedBoundary, Wr = ye.row, rl = ye.debugTask;
                  if (Wr !== null && --Wr.pendingTasks === 0 && Tt(O, Wr), O.allPendingTasks--, re = it(
                    O,
                    zn,
                    Bn,
                    rl
                  ), pt === null)
                    Jl(
                      O,
                      zn,
                      Bn,
                      rl
                    );
                  else if (pt.pendingTasks--, pt.status !== he) {
                    pt.status = he, Pl(
                      pt,
                      re,
                      zn,
                      Bn,
                      !1
                    ), uo(O, pt);
                    var It = pt.row;
                    It !== null && --It.pendingTasks === 0 && Tt(O, It), pt.parentFlushed && O.clientRenderedBoundaries.push(pt), O.pendingRootTasks === 0 && O.trackedPostpones === null && pt.contentPreamble !== null && Ca(O);
                  }
                  O.allPendingTasks === 0 && Xi(O);
                }
              } finally {
                r = te;
              }
            }
          }
          A.splice(0, X), e.destination !== null && Qr(
            e,
            e.destination
          );
        } catch (Zn) {
          A = {}, it(e, Zn, A, null), Jl(e, Zn, A, null);
        } finally {
          n = k, Le.H = c, Le.A = d, Le.getCurrentStack = w, c === yc && $e(t), yn = b;
        }
      }
    }
    function Cu(e, t, c) {
      t.preambleChildren.length && c.push(t.preambleChildren);
      for (var d = !1, b = 0; b < t.children.length; b++)
        d = vs(
          e,
          t.children[b],
          c
        ) || d;
      return d;
    }
    function vs(e, t, c) {
      var d = t.boundary;
      if (d === null)
        return Cu(
          e,
          t,
          c
        );
      var b = d.contentPreamble, w = d.fallbackPreamble;
      if (b === null || w === null) return !1;
      switch (d.status) {
        case bn:
          if (hu(e.renderState, b), e.byteSize += d.byteSize, t = d.completedSegments[0], !t)
            throw Error(
              "A previously unvisited boundary must have exactly one root segment. This is a bug in React."
            );
          return Cu(
            e,
            t,
            c
          );
        case Oe:
          if (e.trackedPostpones !== null) return !0;
        case he:
          if (t.status === bn)
            return hu(e.renderState, w), Cu(
              e,
              t,
              c
            );
        default:
          return !0;
      }
    }
    function Ca(e) {
      if (e.completedRootSegment && e.completedPreambleSegments === null) {
        var t = [], c = e.byteSize, d = vs(
          e,
          e.completedRootSegment,
          t
        ), b = e.renderState.preamble;
        d === !1 || b.headChunks && b.bodyChunks ? e.completedPreambleSegments = t : e.byteSize = c;
      }
    }
    function ot(e, t, c, d) {
      switch (c.parentFlushed = !0, c.status) {
        case vn:
          c.id = e.nextSegmentId++;
        case Oe:
          return d = c.id, c.lastPushedText = !1, c.textEmbedded = !1, e = e.renderState, H(t, dc), H(t, e.placeholderPrefix), e = ne(d.toString(16)), H(t, e), pe(t, Jc);
        case bn:
          c.status = wn;
          var b = !0, w = c.chunks, k = 0;
          c = c.children;
          for (var A = 0; A < c.length; A++) {
            for (b = c[A]; k < b.index; k++)
              H(t, w[k]);
            b = Ql(e, t, b, d);
          }
          for (; k < w.length - 1; k++)
            H(t, w[k]);
          return k < w.length && (b = pe(t, w[k])), b;
        case Xn:
          return !0;
        default:
          throw Error(
            "Aborted, errored or already flushed boundaries should not be flushed again. This is a bug in React."
          );
      }
    }
    function Ql(e, t, c, d) {
      var b = c.boundary;
      if (b === null)
        return ot(e, t, c, d);
      if (b.parentFlushed = !0, b.status === he) {
        var w = b.row;
        w !== null && --w.pendingTasks === 0 && Tt(e, w), w = b.errorDigest;
        var k = b.errorMessage, A = b.errorStack;
        b = b.errorComponentStack, pe(t, ts), H(t, hc), w && (H(t, wo), H(t, ne(ge(w))), H(
          t,
          Ml
        )), k && (H(t, ra), H(
          t,
          ne(ge(k))
        ), H(
          t,
          Ml
        )), A && (H(t, gc), H(
          t,
          ne(ge(A))
        ), H(
          t,
          Ml
        )), b && (H(t, la), H(
          t,
          ne(ge(b))
        ), H(
          t,
          Ml
        )), pe(t, rs), ot(e, t, c, d);
      } else if (b.status !== bn)
        b.status === vn && (b.rootSegmentID = e.nextSegmentId++), 0 < b.completedSegments.length && e.partialBoundaries.push(b), si(
          t,
          e.renderState,
          b.rootSegmentID
        ), d && je(d, b.fallbackState), ot(e, t, c, d);
      else if (!zt && wr(e, b) && (li + b.byteSize > e.progressiveChunkSize || Vu(b.contentState)))
        b.rootSegmentID = e.nextSegmentId++, e.completedBoundaries.push(b), si(
          t,
          e.renderState,
          b.rootSegmentID
        ), ot(e, t, c, d);
      else {
        if (li += b.byteSize, d && je(d, b.contentState), c = b.row, c !== null && wr(e, b) && --c.pendingTasks === 0 && Tt(e, c), pe(t, Qc), c = b.completedSegments, c.length !== 1)
          throw Error(
            "A previously unvisited boundary must have exactly one root segment. This is a bug in React."
          );
        Ql(e, t, c[0], d);
      }
      return pe(t, za);
    }
    function jo(e, t, c, d) {
      return vu(
        t,
        e.renderState,
        c.parentFormatContext,
        c.id
      ), Ql(e, t, c, d), No(t, c.parentFormatContext);
    }
    function $o(e, t, c) {
      li = c.byteSize;
      for (var d = c.completedSegments, b = 0; b < d.length; b++)
        Dc(
          e,
          t,
          c,
          d[b]
        );
      d.length = 0, d = c.row, d !== null && wr(e, c) && --d.pendingTasks === 0 && Tt(e, d), Nn(
        t,
        c.contentState,
        e.renderState
      ), d = e.resumableState, e = e.renderState, b = c.rootSegmentID, c = c.contentState;
      var w = e.stylesToHoist;
      return e.stylesToHoist = !1, H(t, e.startInlineScript), H(t, Un), w ? ((d.instructions & sn) === Hn && (d.instructions |= sn, H(t, or)), (d.instructions & nr) === Hn && (d.instructions |= nr, H(t, Se)), (d.instructions & Ke) === Hn ? (d.instructions |= Ke, H(
        t,
        Yn
      )) : H(t, Fn)) : ((d.instructions & nr) === Hn && (d.instructions |= nr, H(t, Se)), H(t, Xe)), d = ne(b.toString(16)), H(t, e.boundaryPrefix), H(t, d), H(t, Zt), H(t, e.segmentPrefix), H(t, d), w ? (H(t, Dr), Hl(t, c)) : H(t, fl), c = pe(t, Pe), gu(t, e) && c;
    }
    function Dc(e, t, c, d) {
      if (d.status === wn) return !0;
      var b = c.contentState, w = d.id;
      if (w === -1) {
        if ((d.id = c.rootSegmentID) === -1)
          throw Error(
            "A root segment ID must have been assigned by now. This is a bug in React."
          );
        return jo(
          e,
          t,
          d,
          b
        );
      }
      return w === c.rootSegmentID ? jo(
        e,
        t,
        d,
        b
      ) : (jo(e, t, d, b), c = e.resumableState, e = e.renderState, H(t, e.startInlineScript), H(t, Un), (c.instructions & nn) === Hn ? (c.instructions |= nn, H(t, Z)) : H(t, se), H(t, e.segmentPrefix), w = ne(w.toString(16)), H(t, w), H(t, ce), H(t, e.placeholderPrefix), H(t, w), t = pe(t, Ge), t);
    }
    function Qr(e, t) {
      Gt = new Uint8Array(2048), Dt = 0;
      try {
        if (!(0 < e.pendingRootTasks)) {
          var c, d = e.completedRootSegment;
          if (d !== null) {
            if (d.status === Oe) return;
            var b = e.completedPreambleSegments;
            if (b === null) return;
            li = e.byteSize;
            var w = e.resumableState, k = e.renderState, A = k.preamble, X = A.htmlChunks, O = A.headChunks, z;
            if (X) {
              for (z = 0; z < X.length; z++)
                H(t, X[z]);
              if (O)
                for (z = 0; z < O.length; z++)
                  H(t, O[z]);
              else
                H(t, Rn("head")), H(t, Un);
            } else if (O)
              for (z = 0; z < O.length; z++)
                H(t, O[z]);
            var ee = k.charsetChunks;
            for (z = 0; z < ee.length; z++)
              H(t, ee[z]);
            ee.length = 0, k.preconnects.forEach(An, t), k.preconnects.clear();
            var re = k.viewportChunks;
            for (z = 0; z < re.length; z++)
              H(t, re[z]);
            re.length = 0, k.fontPreloads.forEach(An, t), k.fontPreloads.clear(), k.highImagePreloads.forEach(An, t), k.highImagePreloads.clear(), Be = k, k.styles.forEach(va, t), Be = null;
            var te = k.importMapChunks;
            for (z = 0; z < te.length; z++)
              H(t, te[z]);
            te.length = 0, k.bootstrapScripts.forEach(An, t), k.scripts.forEach(An, t), k.scripts.clear(), k.bulkPreloads.forEach(An, t), k.bulkPreloads.clear(), X || O || (w.instructions |= tn);
            var j = k.hoistableChunks;
            for (z = 0; z < j.length; z++)
              H(t, j[z]);
            for (w = j.length = 0; w < b.length; w++) {
              var we = b[w];
              for (k = 0; k < we.length; k++)
                Ql(e, t, we[k], null);
            }
            var pn = e.renderState.preamble, ye = pn.headChunks;
            (pn.htmlChunks || ye) && H(t, et("head"));
            var Ue = pn.bodyChunks;
            if (Ue)
              for (b = 0; b < Ue.length; b++)
                H(t, Ue[b]);
            Ql(e, t, d, null), e.completedRootSegment = null;
            var jn = e.renderState;
            if (e.allPendingTasks !== 0 || e.clientRenderedBoundaries.length !== 0 || e.completedBoundaries.length !== 0 || e.trackedPostpones !== null && (e.trackedPostpones.rootNodes.length !== 0 || e.trackedPostpones.rootSlots !== null)) {
              var ft = e.resumableState;
              if ((ft.instructions & Ae) === Hn) {
                if (ft.instructions |= Ae, H(t, jn.startInlineScript), (ft.instructions & tn) === Hn) {
                  ft.instructions |= tn;
                  var an = "_" + ft.idPrefix + "R_";
                  H(t, Kc), H(
                    t,
                    ne(ge(an))
                  ), H(t, ze);
                }
                H(t, Un), H(t, fc), pe(t, Ei);
              }
            }
            gu(t, jn);
          }
          var be = e.renderState;
          d = 0;
          var dt = be.viewportChunks;
          for (d = 0; d < dt.length; d++)
            H(
              t,
              dt[d]
            );
          dt.length = 0, be.preconnects.forEach(An, t), be.preconnects.clear(), be.fontPreloads.forEach(An, t), be.fontPreloads.clear(), be.highImagePreloads.forEach(
            An,
            t
          ), be.highImagePreloads.clear(), be.styles.forEach(bu, t), be.scripts.forEach(An, t), be.scripts.clear(), be.bulkPreloads.forEach(An, t), be.bulkPreloads.clear();
          var hr = be.hoistableChunks;
          for (d = 0; d < hr.length; d++)
            H(
              t,
              hr[d]
            );
          hr.length = 0;
          var En = e.clientRenderedBoundaries;
          for (c = 0; c < En.length; c++) {
            var zn = En[c];
            be = t;
            var Nr = e.resumableState, Kt = e.renderState, At = zn.rootSegmentID, Bn = zn.errorDigest, pt = zn.errorMessage, Wr = zn.errorStack, rl = zn.errorComponentStack;
            H(
              be,
              Kt.startInlineScript
            ), H(be, Un), (Nr.instructions & sn) === Hn ? (Nr.instructions |= sn, H(be, tt)) : H(be, kr), H(
              be,
              Kt.boundaryPrefix
            ), H(be, ne(At.toString(16))), H(be, Pi), (Bn || pt || Wr || rl) && (H(
              be,
              Gn
            ), H(
              be,
              ne(
                ha(Bn || "")
              )
            )), (pt || Wr || rl) && (H(
              be,
              Gn
            ), H(
              be,
              ne(
                ha(pt || "")
              )
            )), (Wr || rl) && (H(
              be,
              Gn
            ), H(
              be,
              ne(
                ha(Wr || "")
              )
            )), rl && (H(
              be,
              Gn
            ), H(
              be,
              ne(
                ha(rl)
              )
            ));
            var It = pe(
              be,
              Na
            );
            if (!It) {
              e.destination = null, c++, En.splice(0, c);
              return;
            }
          }
          En.splice(0, c);
          var Zn = e.completedBoundaries;
          for (c = 0; c < Zn.length; c++)
            if (!$o(
              e,
              t,
              Zn[c]
            )) {
              e.destination = null, c++, Zn.splice(0, c);
              return;
            }
          Zn.splice(0, c), Ar(t), Gt = new Uint8Array(2048), Dt = 0, zt = !0;
          var gr = e.partialBoundaries;
          for (c = 0; c < gr.length; c++) {
            e: {
              En = e, zn = t;
              var Hr = gr[c];
              li = Hr.byteSize;
              var vr = Hr.completedSegments;
              for (It = 0; It < vr.length; It++)
                if (!Dc(
                  En,
                  zn,
                  Hr,
                  vr[It]
                )) {
                  It++, vr.splice(0, It);
                  var Ii = !1;
                  break e;
                }
              vr.splice(0, It);
              var Ur = Hr.row;
              Ur !== null && Ur.together && Hr.pendingTasks === 1 && (Ur.pendingTasks === 1 ? Ui(
                En,
                Ur,
                Ur.hoistables
              ) : Ur.pendingTasks--), Ii = Nn(
                zn,
                Hr.contentState,
                En.renderState
              );
            }
            if (!Ii) {
              e.destination = null, c++, gr.splice(0, c);
              return;
            }
          }
          gr.splice(0, c), zt = !1;
          var ht = e.completedBoundaries;
          for (c = 0; c < ht.length; c++)
            if (!$o(e, t, ht[c])) {
              e.destination = null, c++, ht.splice(0, c);
              return;
            }
          ht.splice(0, c);
        }
      } finally {
        zt = !1, e.allPendingTasks === 0 && e.clientRenderedBoundaries.length === 0 && e.completedBoundaries.length === 0 ? (e.flushScheduled = !1, c = e.resumableState, c.hasBody && H(t, et("body")), c.hasHtml && H(t, et("html")), Ar(t), e.abortableTasks.size !== 0 && console.error(
          "There was still abortable task at the root when we closed. This is a bug in React."
        ), e.status = dr, t.close(), e.destination = null) : Ar(t);
      }
    }
    function Kr(e) {
      e.flushScheduled = e.destination !== null, Fa(function() {
        return Ji(e);
      }), Et(function() {
        e.status === 10 && (e.status = 11), e.trackedPostpones === null && _c(e, e.pendingRootTasks === 0);
      });
    }
    function _r(e) {
      e.flushScheduled === !1 && e.pingedTasks.length === 0 && e.destination !== null && (e.flushScheduled = !0, Et(function() {
        var t = e.destination;
        t ? Qr(e, t) : e.flushScheduled = !1;
      }));
    }
    function qr(e, t) {
      if (e.status === 13)
        e.status = dr, zo(t, e.fatalError);
      else if (e.status !== dr && e.destination === null) {
        e.destination = t;
        try {
          Qr(e, t);
        } catch (c) {
          t = {}, it(e, c, t, null), Jl(e, c, t, null);
        }
      }
    }
    function Tn(e, t) {
      (e.status === 11 || e.status === 10) && (e.status = 12);
      try {
        var c = e.abortableTasks;
        if (0 < c.size) {
          var d = t === void 0 ? Error("The render was aborted by the server without a reason.") : typeof t == "object" && t !== null && typeof t.then == "function" ? Error("The render was aborted by the server with a promise.") : t;
          e.fatalError = d, c.forEach(function(b) {
            var w = r, k = Le.getCurrentStack;
            r = b, Le.getCurrentStack = Zl;
            try {
              qo(b, e, d);
            } finally {
              r = w, Le.getCurrentStack = k;
            }
          }), c.clear();
        }
        e.destination !== null && Qr(e, e.destination);
      } catch (b) {
        t = {}, it(e, b, t, null), Jl(e, b, t, null);
      }
    }
    function jr(e, t, c) {
      if (t === null) c.rootNodes.push(e);
      else {
        var d = c.workingMap, b = d.get(t);
        b === void 0 && (b = [t[1], t[2], [], null], d.set(t, b), jr(b, t[0], c)), b[2].push(e);
      }
    }
    function Mn(e) {
      var t = e.trackedPostpones;
      if (t === null || t.rootNodes.length === 0 && t.rootSlots === null)
        return e.trackedPostpones = null;
      if (e.completedRootSegment === null || e.completedRootSegment.status !== Oe && e.completedPreambleSegments !== null) {
        var c = e.nextSegmentId, d = t.rootSlots, b = e.resumableState;
        b.bootstrapScriptContent = void 0, b.bootstrapScripts = void 0, b.bootstrapModules = void 0;
      } else {
        c = 0, d = -1, b = e.resumableState;
        var w = e.renderState;
        b.nextFormID = 0, b.hasBody = !1, b.hasHtml = !1, b.unknownResources = { font: w.resets.font }, b.dnsResources = w.resets.dns, b.connectResources = w.resets.connect, b.imageResources = w.resets.image, b.styleResources = w.resets.style, b.scriptResources = {}, b.moduleUnknownResources = {}, b.moduleScriptResources = {}, b.instructions = Hn;
      }
      return {
        nextSegmentId: c,
        rootFormatContext: e.rootFormatContext,
        progressiveChunkSize: e.progressiveChunkSize,
        resumableState: e.resumableState,
        replayNodes: t.rootNodes,
        replaySlots: d
      };
    }
    function Al() {
      var e = Lc.version;
      if (e !== "19.2.4")
        throw Error(
          `Incompatible React versions: The "react" and "react-dom" packages must have the exact same version. Instead got:
  - react:      ` + (e + `
  - react-dom:  19.2.4
Learn more: https://react.dev/warnings/version-mismatch`)
        );
    }
    var Lc = ks, zc = rf, Bc = Symbol.for("react.transitional.element"), ec = Symbol.for("react.portal"), Ti = Symbol.for("react.fragment"), nc = Symbol.for("react.strict_mode"), tc = Symbol.for("react.profiler"), Rr = Symbol.for("react.consumer"), On = Symbol.for("react.context"), en = Symbol.for("react.forward_ref"), Vi = Symbol.for("react.suspense"), ka = Symbol.for("react.suspense_list"), Sa = Symbol.for("react.memo"), Qi = Symbol.for("react.lazy"), ku = Symbol.for("react.scope"), _t = Symbol.for("react.activity"), bs = Symbol.for("react.legacy_hidden"), Il = Symbol.for("react.memo_cache_sentinel"), $u = Symbol.for("react.view_transition"), Su = Symbol.iterator, Kl = Array.isArray, rc = /* @__PURE__ */ new WeakMap(), Yt = /* @__PURE__ */ new WeakMap(), wi = Symbol.for("react.client.reference"), es = new MessageChannel(), Pa = [];
    es.port1.onmessage = function() {
      var e = Pa.shift();
      e && e();
    };
    var Nc = Promise, Fa = typeof queueMicrotask == "function" ? queueMicrotask : function(e) {
      Nc.resolve(null).then(e).catch(jt);
    }, Gt = null, Dt = 0, Ki = new TextEncoder(), Pn = Object.assign, gn = Object.prototype.hasOwnProperty, ma = RegExp(
      "^[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), go = {}, Wc = {}, vo = new Set(
      "animationIterationCount aspectRatio borderImageOutset borderImageSlice borderImageWidth boxFlex boxFlexGroup boxOrdinalGroup columnCount columns flex flexGrow flexPositive flexShrink flexNegative flexOrder gridArea gridRow gridRowEnd gridRowSpan gridRowStart gridColumn gridColumnEnd gridColumnSpan gridColumnStart fontWeight lineClamp lineHeight opacity order orphans scale tabSize widows zIndex zoom fillOpacity floodOpacity stopOpacity strokeDasharray strokeDashoffset strokeMiterlimit strokeOpacity strokeWidth MozAnimationIterationCount MozBoxFlex MozBoxFlexGroup MozLineClamp msAnimationIterationCount msFlex msZoom msFlexGrow msFlexNegative msFlexOrder msFlexPositive msFlexShrink msGridColumn msGridColumnSpan msGridRow msGridRowSpan WebkitAnimationIterationCount WebkitBoxFlex WebKitBoxFlexGroup WebkitBoxOrdinalGroup WebkitColumnCount WebkitColumns WebkitFlex WebkitFlexGrow WebkitFlexPositive WebkitFlexShrink WebkitLineClamp".split(
        " "
      )
    ), Pu = /* @__PURE__ */ new Map([
      ["acceptCharset", "accept-charset"],
      ["htmlFor", "for"],
      ["httpEquiv", "http-equiv"],
      ["crossOrigin", "crossorigin"],
      ["accentHeight", "accent-height"],
      ["alignmentBaseline", "alignment-baseline"],
      ["arabicForm", "arabic-form"],
      ["baselineShift", "baseline-shift"],
      ["capHeight", "cap-height"],
      ["clipPath", "clip-path"],
      ["clipRule", "clip-rule"],
      ["colorInterpolation", "color-interpolation"],
      ["colorInterpolationFilters", "color-interpolation-filters"],
      ["colorProfile", "color-profile"],
      ["colorRendering", "color-rendering"],
      ["dominantBaseline", "dominant-baseline"],
      ["enableBackground", "enable-background"],
      ["fillOpacity", "fill-opacity"],
      ["fillRule", "fill-rule"],
      ["floodColor", "flood-color"],
      ["floodOpacity", "flood-opacity"],
      ["fontFamily", "font-family"],
      ["fontSize", "font-size"],
      ["fontSizeAdjust", "font-size-adjust"],
      ["fontStretch", "font-stretch"],
      ["fontStyle", "font-style"],
      ["fontVariant", "font-variant"],
      ["fontWeight", "font-weight"],
      ["glyphName", "glyph-name"],
      ["glyphOrientationHorizontal", "glyph-orientation-horizontal"],
      ["glyphOrientationVertical", "glyph-orientation-vertical"],
      ["horizAdvX", "horiz-adv-x"],
      ["horizOriginX", "horiz-origin-x"],
      ["imageRendering", "image-rendering"],
      ["letterSpacing", "letter-spacing"],
      ["lightingColor", "lighting-color"],
      ["markerEnd", "marker-end"],
      ["markerMid", "marker-mid"],
      ["markerStart", "marker-start"],
      ["overlinePosition", "overline-position"],
      ["overlineThickness", "overline-thickness"],
      ["paintOrder", "paint-order"],
      ["panose-1", "panose-1"],
      ["pointerEvents", "pointer-events"],
      ["renderingIntent", "rendering-intent"],
      ["shapeRendering", "shape-rendering"],
      ["stopColor", "stop-color"],
      ["stopOpacity", "stop-opacity"],
      ["strikethroughPosition", "strikethrough-position"],
      ["strikethroughThickness", "strikethrough-thickness"],
      ["strokeDasharray", "stroke-dasharray"],
      ["strokeDashoffset", "stroke-dashoffset"],
      ["strokeLinecap", "stroke-linecap"],
      ["strokeLinejoin", "stroke-linejoin"],
      ["strokeMiterlimit", "stroke-miterlimit"],
      ["strokeOpacity", "stroke-opacity"],
      ["strokeWidth", "stroke-width"],
      ["textAnchor", "text-anchor"],
      ["textDecoration", "text-decoration"],
      ["textRendering", "text-rendering"],
      ["transformOrigin", "transform-origin"],
      ["underlinePosition", "underline-position"],
      ["underlineThickness", "underline-thickness"],
      ["unicodeBidi", "unicode-bidi"],
      ["unicodeRange", "unicode-range"],
      ["unitsPerEm", "units-per-em"],
      ["vAlphabetic", "v-alphabetic"],
      ["vHanging", "v-hanging"],
      ["vIdeographic", "v-ideographic"],
      ["vMathematical", "v-mathematical"],
      ["vectorEffect", "vector-effect"],
      ["vertAdvY", "vert-adv-y"],
      ["vertOriginX", "vert-origin-x"],
      ["vertOriginY", "vert-origin-y"],
      ["wordSpacing", "word-spacing"],
      ["writingMode", "writing-mode"],
      ["xmlnsXlink", "xmlns:xlink"],
      ["xHeight", "x-height"]
    ]), ys = {
      button: !0,
      checkbox: !0,
      image: !0,
      hidden: !0,
      radio: !0,
      reset: !0,
      submit: !0
    }, Wn = {
      "aria-current": 0,
      "aria-description": 0,
      "aria-details": 0,
      "aria-disabled": 0,
      "aria-hidden": 0,
      "aria-invalid": 0,
      "aria-keyshortcuts": 0,
      "aria-label": 0,
      "aria-roledescription": 0,
      "aria-autocomplete": 0,
      "aria-checked": 0,
      "aria-expanded": 0,
      "aria-haspopup": 0,
      "aria-level": 0,
      "aria-modal": 0,
      "aria-multiline": 0,
      "aria-multiselectable": 0,
      "aria-orientation": 0,
      "aria-placeholder": 0,
      "aria-pressed": 0,
      "aria-readonly": 0,
      "aria-required": 0,
      "aria-selected": 0,
      "aria-sort": 0,
      "aria-valuemax": 0,
      "aria-valuemin": 0,
      "aria-valuenow": 0,
      "aria-valuetext": 0,
      "aria-atomic": 0,
      "aria-busy": 0,
      "aria-live": 0,
      "aria-relevant": 0,
      "aria-dropeffect": 0,
      "aria-grabbed": 0,
      "aria-activedescendant": 0,
      "aria-colcount": 0,
      "aria-colindex": 0,
      "aria-colspan": 0,
      "aria-controls": 0,
      "aria-describedby": 0,
      "aria-errormessage": 0,
      "aria-flowto": 0,
      "aria-labelledby": 0,
      "aria-owns": 0,
      "aria-posinset": 0,
      "aria-rowcount": 0,
      "aria-rowindex": 0,
      "aria-rowspan": 0,
      "aria-setsize": 0,
      "aria-braillelabel": 0,
      "aria-brailleroledescription": 0,
      "aria-colindextext": 0,
      "aria-rowindextext": 0
    }, pi = {}, ns = RegExp(
      "^(aria)-[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), i = RegExp(
      "^(aria)[A-Z][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), o = !1, f = {
      accept: "accept",
      acceptcharset: "acceptCharset",
      "accept-charset": "acceptCharset",
      accesskey: "accessKey",
      action: "action",
      allowfullscreen: "allowFullScreen",
      alt: "alt",
      as: "as",
      async: "async",
      autocapitalize: "autoCapitalize",
      autocomplete: "autoComplete",
      autocorrect: "autoCorrect",
      autofocus: "autoFocus",
      autoplay: "autoPlay",
      autosave: "autoSave",
      capture: "capture",
      cellpadding: "cellPadding",
      cellspacing: "cellSpacing",
      challenge: "challenge",
      charset: "charSet",
      checked: "checked",
      children: "children",
      cite: "cite",
      class: "className",
      classid: "classID",
      classname: "className",
      cols: "cols",
      colspan: "colSpan",
      content: "content",
      contenteditable: "contentEditable",
      contextmenu: "contextMenu",
      controls: "controls",
      controlslist: "controlsList",
      coords: "coords",
      crossorigin: "crossOrigin",
      dangerouslysetinnerhtml: "dangerouslySetInnerHTML",
      data: "data",
      datetime: "dateTime",
      default: "default",
      defaultchecked: "defaultChecked",
      defaultvalue: "defaultValue",
      defer: "defer",
      dir: "dir",
      disabled: "disabled",
      disablepictureinpicture: "disablePictureInPicture",
      disableremoteplayback: "disableRemotePlayback",
      download: "download",
      draggable: "draggable",
      enctype: "encType",
      enterkeyhint: "enterKeyHint",
      fetchpriority: "fetchPriority",
      for: "htmlFor",
      form: "form",
      formmethod: "formMethod",
      formaction: "formAction",
      formenctype: "formEncType",
      formnovalidate: "formNoValidate",
      formtarget: "formTarget",
      frameborder: "frameBorder",
      headers: "headers",
      height: "height",
      hidden: "hidden",
      high: "high",
      href: "href",
      hreflang: "hrefLang",
      htmlfor: "htmlFor",
      httpequiv: "httpEquiv",
      "http-equiv": "httpEquiv",
      icon: "icon",
      id: "id",
      imagesizes: "imageSizes",
      imagesrcset: "imageSrcSet",
      inert: "inert",
      innerhtml: "innerHTML",
      inputmode: "inputMode",
      integrity: "integrity",
      is: "is",
      itemid: "itemID",
      itemprop: "itemProp",
      itemref: "itemRef",
      itemscope: "itemScope",
      itemtype: "itemType",
      keyparams: "keyParams",
      keytype: "keyType",
      kind: "kind",
      label: "label",
      lang: "lang",
      list: "list",
      loop: "loop",
      low: "low",
      manifest: "manifest",
      marginwidth: "marginWidth",
      marginheight: "marginHeight",
      max: "max",
      maxlength: "maxLength",
      media: "media",
      mediagroup: "mediaGroup",
      method: "method",
      min: "min",
      minlength: "minLength",
      multiple: "multiple",
      muted: "muted",
      name: "name",
      nomodule: "noModule",
      nonce: "nonce",
      novalidate: "noValidate",
      open: "open",
      optimum: "optimum",
      pattern: "pattern",
      placeholder: "placeholder",
      playsinline: "playsInline",
      poster: "poster",
      preload: "preload",
      profile: "profile",
      radiogroup: "radioGroup",
      readonly: "readOnly",
      referrerpolicy: "referrerPolicy",
      rel: "rel",
      required: "required",
      reversed: "reversed",
      role: "role",
      rows: "rows",
      rowspan: "rowSpan",
      sandbox: "sandbox",
      scope: "scope",
      scoped: "scoped",
      scrolling: "scrolling",
      seamless: "seamless",
      selected: "selected",
      shape: "shape",
      size: "size",
      sizes: "sizes",
      span: "span",
      spellcheck: "spellCheck",
      src: "src",
      srcdoc: "srcDoc",
      srclang: "srcLang",
      srcset: "srcSet",
      start: "start",
      step: "step",
      style: "style",
      summary: "summary",
      tabindex: "tabIndex",
      target: "target",
      title: "title",
      type: "type",
      usemap: "useMap",
      value: "value",
      width: "width",
      wmode: "wmode",
      wrap: "wrap",
      about: "about",
      accentheight: "accentHeight",
      "accent-height": "accentHeight",
      accumulate: "accumulate",
      additive: "additive",
      alignmentbaseline: "alignmentBaseline",
      "alignment-baseline": "alignmentBaseline",
      allowreorder: "allowReorder",
      alphabetic: "alphabetic",
      amplitude: "amplitude",
      arabicform: "arabicForm",
      "arabic-form": "arabicForm",
      ascent: "ascent",
      attributename: "attributeName",
      attributetype: "attributeType",
      autoreverse: "autoReverse",
      azimuth: "azimuth",
      basefrequency: "baseFrequency",
      baselineshift: "baselineShift",
      "baseline-shift": "baselineShift",
      baseprofile: "baseProfile",
      bbox: "bbox",
      begin: "begin",
      bias: "bias",
      by: "by",
      calcmode: "calcMode",
      capheight: "capHeight",
      "cap-height": "capHeight",
      clip: "clip",
      clippath: "clipPath",
      "clip-path": "clipPath",
      clippathunits: "clipPathUnits",
      cliprule: "clipRule",
      "clip-rule": "clipRule",
      color: "color",
      colorinterpolation: "colorInterpolation",
      "color-interpolation": "colorInterpolation",
      colorinterpolationfilters: "colorInterpolationFilters",
      "color-interpolation-filters": "colorInterpolationFilters",
      colorprofile: "colorProfile",
      "color-profile": "colorProfile",
      colorrendering: "colorRendering",
      "color-rendering": "colorRendering",
      contentscripttype: "contentScriptType",
      contentstyletype: "contentStyleType",
      cursor: "cursor",
      cx: "cx",
      cy: "cy",
      d: "d",
      datatype: "datatype",
      decelerate: "decelerate",
      descent: "descent",
      diffuseconstant: "diffuseConstant",
      direction: "direction",
      display: "display",
      divisor: "divisor",
      dominantbaseline: "dominantBaseline",
      "dominant-baseline": "dominantBaseline",
      dur: "dur",
      dx: "dx",
      dy: "dy",
      edgemode: "edgeMode",
      elevation: "elevation",
      enablebackground: "enableBackground",
      "enable-background": "enableBackground",
      end: "end",
      exponent: "exponent",
      externalresourcesrequired: "externalResourcesRequired",
      fill: "fill",
      fillopacity: "fillOpacity",
      "fill-opacity": "fillOpacity",
      fillrule: "fillRule",
      "fill-rule": "fillRule",
      filter: "filter",
      filterres: "filterRes",
      filterunits: "filterUnits",
      floodopacity: "floodOpacity",
      "flood-opacity": "floodOpacity",
      floodcolor: "floodColor",
      "flood-color": "floodColor",
      focusable: "focusable",
      fontfamily: "fontFamily",
      "font-family": "fontFamily",
      fontsize: "fontSize",
      "font-size": "fontSize",
      fontsizeadjust: "fontSizeAdjust",
      "font-size-adjust": "fontSizeAdjust",
      fontstretch: "fontStretch",
      "font-stretch": "fontStretch",
      fontstyle: "fontStyle",
      "font-style": "fontStyle",
      fontvariant: "fontVariant",
      "font-variant": "fontVariant",
      fontweight: "fontWeight",
      "font-weight": "fontWeight",
      format: "format",
      from: "from",
      fx: "fx",
      fy: "fy",
      g1: "g1",
      g2: "g2",
      glyphname: "glyphName",
      "glyph-name": "glyphName",
      glyphorientationhorizontal: "glyphOrientationHorizontal",
      "glyph-orientation-horizontal": "glyphOrientationHorizontal",
      glyphorientationvertical: "glyphOrientationVertical",
      "glyph-orientation-vertical": "glyphOrientationVertical",
      glyphref: "glyphRef",
      gradienttransform: "gradientTransform",
      gradientunits: "gradientUnits",
      hanging: "hanging",
      horizadvx: "horizAdvX",
      "horiz-adv-x": "horizAdvX",
      horizoriginx: "horizOriginX",
      "horiz-origin-x": "horizOriginX",
      ideographic: "ideographic",
      imagerendering: "imageRendering",
      "image-rendering": "imageRendering",
      in2: "in2",
      in: "in",
      inlist: "inlist",
      intercept: "intercept",
      k1: "k1",
      k2: "k2",
      k3: "k3",
      k4: "k4",
      k: "k",
      kernelmatrix: "kernelMatrix",
      kernelunitlength: "kernelUnitLength",
      kerning: "kerning",
      keypoints: "keyPoints",
      keysplines: "keySplines",
      keytimes: "keyTimes",
      lengthadjust: "lengthAdjust",
      letterspacing: "letterSpacing",
      "letter-spacing": "letterSpacing",
      lightingcolor: "lightingColor",
      "lighting-color": "lightingColor",
      limitingconeangle: "limitingConeAngle",
      local: "local",
      markerend: "markerEnd",
      "marker-end": "markerEnd",
      markerheight: "markerHeight",
      markermid: "markerMid",
      "marker-mid": "markerMid",
      markerstart: "markerStart",
      "marker-start": "markerStart",
      markerunits: "markerUnits",
      markerwidth: "markerWidth",
      mask: "mask",
      maskcontentunits: "maskContentUnits",
      maskunits: "maskUnits",
      mathematical: "mathematical",
      mode: "mode",
      numoctaves: "numOctaves",
      offset: "offset",
      opacity: "opacity",
      operator: "operator",
      order: "order",
      orient: "orient",
      orientation: "orientation",
      origin: "origin",
      overflow: "overflow",
      overlineposition: "overlinePosition",
      "overline-position": "overlinePosition",
      overlinethickness: "overlineThickness",
      "overline-thickness": "overlineThickness",
      paintorder: "paintOrder",
      "paint-order": "paintOrder",
      panose1: "panose1",
      "panose-1": "panose1",
      pathlength: "pathLength",
      patterncontentunits: "patternContentUnits",
      patterntransform: "patternTransform",
      patternunits: "patternUnits",
      pointerevents: "pointerEvents",
      "pointer-events": "pointerEvents",
      points: "points",
      pointsatx: "pointsAtX",
      pointsaty: "pointsAtY",
      pointsatz: "pointsAtZ",
      popover: "popover",
      popovertarget: "popoverTarget",
      popovertargetaction: "popoverTargetAction",
      prefix: "prefix",
      preservealpha: "preserveAlpha",
      preserveaspectratio: "preserveAspectRatio",
      primitiveunits: "primitiveUnits",
      property: "property",
      r: "r",
      radius: "radius",
      refx: "refX",
      refy: "refY",
      renderingintent: "renderingIntent",
      "rendering-intent": "renderingIntent",
      repeatcount: "repeatCount",
      repeatdur: "repeatDur",
      requiredextensions: "requiredExtensions",
      requiredfeatures: "requiredFeatures",
      resource: "resource",
      restart: "restart",
      result: "result",
      results: "results",
      rotate: "rotate",
      rx: "rx",
      ry: "ry",
      scale: "scale",
      security: "security",
      seed: "seed",
      shaperendering: "shapeRendering",
      "shape-rendering": "shapeRendering",
      slope: "slope",
      spacing: "spacing",
      specularconstant: "specularConstant",
      specularexponent: "specularExponent",
      speed: "speed",
      spreadmethod: "spreadMethod",
      startoffset: "startOffset",
      stddeviation: "stdDeviation",
      stemh: "stemh",
      stemv: "stemv",
      stitchtiles: "stitchTiles",
      stopcolor: "stopColor",
      "stop-color": "stopColor",
      stopopacity: "stopOpacity",
      "stop-opacity": "stopOpacity",
      strikethroughposition: "strikethroughPosition",
      "strikethrough-position": "strikethroughPosition",
      strikethroughthickness: "strikethroughThickness",
      "strikethrough-thickness": "strikethroughThickness",
      string: "string",
      stroke: "stroke",
      strokedasharray: "strokeDasharray",
      "stroke-dasharray": "strokeDasharray",
      strokedashoffset: "strokeDashoffset",
      "stroke-dashoffset": "strokeDashoffset",
      strokelinecap: "strokeLinecap",
      "stroke-linecap": "strokeLinecap",
      strokelinejoin: "strokeLinejoin",
      "stroke-linejoin": "strokeLinejoin",
      strokemiterlimit: "strokeMiterlimit",
      "stroke-miterlimit": "strokeMiterlimit",
      strokewidth: "strokeWidth",
      "stroke-width": "strokeWidth",
      strokeopacity: "strokeOpacity",
      "stroke-opacity": "strokeOpacity",
      suppresscontenteditablewarning: "suppressContentEditableWarning",
      suppresshydrationwarning: "suppressHydrationWarning",
      surfacescale: "surfaceScale",
      systemlanguage: "systemLanguage",
      tablevalues: "tableValues",
      targetx: "targetX",
      targety: "targetY",
      textanchor: "textAnchor",
      "text-anchor": "textAnchor",
      textdecoration: "textDecoration",
      "text-decoration": "textDecoration",
      textlength: "textLength",
      textrendering: "textRendering",
      "text-rendering": "textRendering",
      to: "to",
      transform: "transform",
      transformorigin: "transformOrigin",
      "transform-origin": "transformOrigin",
      typeof: "typeof",
      u1: "u1",
      u2: "u2",
      underlineposition: "underlinePosition",
      "underline-position": "underlinePosition",
      underlinethickness: "underlineThickness",
      "underline-thickness": "underlineThickness",
      unicode: "unicode",
      unicodebidi: "unicodeBidi",
      "unicode-bidi": "unicodeBidi",
      unicoderange: "unicodeRange",
      "unicode-range": "unicodeRange",
      unitsperem: "unitsPerEm",
      "units-per-em": "unitsPerEm",
      unselectable: "unselectable",
      valphabetic: "vAlphabetic",
      "v-alphabetic": "vAlphabetic",
      values: "values",
      vectoreffect: "vectorEffect",
      "vector-effect": "vectorEffect",
      version: "version",
      vertadvy: "vertAdvY",
      "vert-adv-y": "vertAdvY",
      vertoriginx: "vertOriginX",
      "vert-origin-x": "vertOriginX",
      vertoriginy: "vertOriginY",
      "vert-origin-y": "vertOriginY",
      vhanging: "vHanging",
      "v-hanging": "vHanging",
      videographic: "vIdeographic",
      "v-ideographic": "vIdeographic",
      viewbox: "viewBox",
      viewtarget: "viewTarget",
      visibility: "visibility",
      vmathematical: "vMathematical",
      "v-mathematical": "vMathematical",
      vocab: "vocab",
      widths: "widths",
      wordspacing: "wordSpacing",
      "word-spacing": "wordSpacing",
      writingmode: "writingMode",
      "writing-mode": "writingMode",
      x1: "x1",
      x2: "x2",
      x: "x",
      xchannelselector: "xChannelSelector",
      xheight: "xHeight",
      "x-height": "xHeight",
      xlinkactuate: "xlinkActuate",
      "xlink:actuate": "xlinkActuate",
      xlinkarcrole: "xlinkArcrole",
      "xlink:arcrole": "xlinkArcrole",
      xlinkhref: "xlinkHref",
      "xlink:href": "xlinkHref",
      xlinkrole: "xlinkRole",
      "xlink:role": "xlinkRole",
      xlinkshow: "xlinkShow",
      "xlink:show": "xlinkShow",
      xlinktitle: "xlinkTitle",
      "xlink:title": "xlinkTitle",
      xlinktype: "xlinkType",
      "xlink:type": "xlinkType",
      xmlbase: "xmlBase",
      "xml:base": "xmlBase",
      xmllang: "xmlLang",
      "xml:lang": "xmlLang",
      xmlns: "xmlns",
      "xml:space": "xmlSpace",
      xmlnsxlink: "xmlnsXlink",
      "xmlns:xlink": "xmlnsXlink",
      xmlspace: "xmlSpace",
      y1: "y1",
      y2: "y2",
      y: "y",
      ychannelselector: "yChannelSelector",
      z: "z",
      zoomandpan: "zoomAndPan"
    }, g = {}, T = /^on./, R = /^on[^A-Z]/, S = RegExp(
      "^(aria)-[:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), Y = RegExp(
      "^(aria)[A-Z][:A-Z_a-z\\u00C0-\\u00D6\\u00D8-\\u00F6\\u00F8-\\u02FF\\u0370-\\u037D\\u037F-\\u1FFF\\u200C-\\u200D\\u2070-\\u218F\\u2C00-\\u2FEF\\u3001-\\uD7FF\\uF900-\\uFDCF\\uFDF0-\\uFFFD\\-.0-9\\u00B7\\u0300-\\u036F\\u203F-\\u2040]*$"
    ), I = /^(?:webkit|moz|o)[A-Z]/, W = /^-ms-/, q = /-(.)/g, Q = /;\s*$/, ae = {}, ve = {}, De = !1, ke = !1, xe = /["'&<>]/, me = /([A-Z])/g, Ce = /^ms-/, _n = /^[\u0000-\u001F ]*j[\r\n\t]*a[\r\n\t]*v[\r\n\t]*a[\r\n\t]*s[\r\n\t]*c[\r\n\t]*r[\r\n\t]*i[\r\n\t]*p[\r\n\t]*t[\r\n\t]*:/i, Le = Lc.__CLIENT_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, Je = zc.__DOM_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, on = Object.freeze({
      pending: !1,
      data: null,
      method: null,
      action: null
    }), Me = Je.d;
    Je.d = {
      f: Me.f,
      r: Me.r,
      D: function(e) {
        var t = yn || null;
        if (t) {
          var c = t.resumableState, d = t.renderState;
          if (typeof e == "string" && e) {
            if (!c.dnsResources.hasOwnProperty(e)) {
              c.dnsResources[e] = rn, c = d.headers;
              var b, w;
              (w = c && 0 < c.remainingCapacity) && (w = (b = "<" + ya(e) + ">; rel=dns-prefetch", 0 <= (c.remainingCapacity -= b.length + 2))), w ? (d.resets.dns[e] = rn, c.preconnects && (c.preconnects += ", "), c.preconnects += b) : (b = [], xr(b, { href: e, rel: "dns-prefetch" }), d.preconnects.add(b));
            }
            _r(t);
          }
        } else Me.D(e);
      },
      C: function(e, t) {
        var c = yn || null;
        if (c) {
          var d = c.resumableState, b = c.renderState;
          if (typeof e == "string" && e) {
            var w = t === "use-credentials" ? "credentials" : typeof t == "string" ? "anonymous" : "default";
            if (!d.connectResources[w].hasOwnProperty(e)) {
              d.connectResources[w][e] = rn, d = b.headers;
              var k, A;
              if (A = d && 0 < d.remainingCapacity) {
                if (A = "<" + ya(e) + ">; rel=preconnect", typeof t == "string") {
                  var X = Or(
                    t,
                    "crossOrigin"
                  );
                  A += '; crossorigin="' + X + '"';
                }
                A = (k = A, 0 <= (d.remainingCapacity -= k.length + 2));
              }
              A ? (b.resets.connect[w][e] = rn, d.preconnects && (d.preconnects += ", "), d.preconnects += k) : (w = [], xr(w, {
                rel: "preconnect",
                href: e,
                crossOrigin: t
              }), b.preconnects.add(w));
            }
            _r(c);
          }
        } else Me.C(e, t);
      },
      L: function(e, t, c) {
        var d = yn || null;
        if (d) {
          var b = d.resumableState, w = d.renderState;
          if (t && e) {
            switch (t) {
              case "image":
                if (c)
                  var k = c.imageSrcSet, A = c.imageSizes, X = c.fetchPriority;
                var O = k ? k + `
` + (A || "") : e;
                if (b.imageResources.hasOwnProperty(O)) return;
                b.imageResources[O] = Ft, b = w.headers;
                var z;
                b && 0 < b.remainingCapacity && typeof k != "string" && X === "high" && (z = Wi(e, t, c), 0 <= (b.remainingCapacity -= z.length + 2)) ? (w.resets.image[O] = Ft, b.highImagePreloads && (b.highImagePreloads += ", "), b.highImagePreloads += z) : (b = [], xr(
                  b,
                  Pn(
                    {
                      rel: "preload",
                      href: k ? void 0 : e,
                      as: t
                    },
                    c
                  )
                ), X === "high" ? w.highImagePreloads.add(b) : (w.bulkPreloads.add(b), w.preloads.images.set(O, b)));
                break;
              case "style":
                if (b.styleResources.hasOwnProperty(e)) return;
                k = [], xr(
                  k,
                  Pn({ rel: "preload", href: e, as: t }, c)
                ), b.styleResources[e] = !c || typeof c.crossOrigin != "string" && typeof c.integrity != "string" ? Ft : [c.crossOrigin, c.integrity], w.preloads.stylesheets.set(e, k), w.bulkPreloads.add(k);
                break;
              case "script":
                if (b.scriptResources.hasOwnProperty(e)) return;
                k = [], w.preloads.scripts.set(e, k), w.bulkPreloads.add(k), xr(
                  k,
                  Pn({ rel: "preload", href: e, as: t }, c)
                ), b.scriptResources[e] = !c || typeof c.crossOrigin != "string" && typeof c.integrity != "string" ? Ft : [c.crossOrigin, c.integrity];
                break;
              default:
                if (b.unknownResources.hasOwnProperty(t)) {
                  if (k = b.unknownResources[t], k.hasOwnProperty(e))
                    return;
                } else
                  k = {}, b.unknownResources[t] = k;
                if (k[e] = Ft, (b = w.headers) && 0 < b.remainingCapacity && t === "font" && (O = Wi(e, t, c), 0 <= (b.remainingCapacity -= O.length + 2)))
                  w.resets.font[e] = Ft, b.fontPreloads && (b.fontPreloads += ", "), b.fontPreloads += O;
                else
                  switch (b = [], e = Pn(
                    { rel: "preload", href: e, as: t },
                    c
                  ), xr(b, e), t) {
                    case "font":
                      w.fontPreloads.add(b);
                      break;
                    default:
                      w.bulkPreloads.add(b);
                  }
            }
            _r(d);
          }
        } else Me.L(e, t, c);
      },
      m: function(e, t) {
        var c = yn || null;
        if (c) {
          var d = c.resumableState, b = c.renderState;
          if (e) {
            var w = t && typeof t.as == "string" ? t.as : "script";
            switch (w) {
              case "script":
                if (d.moduleScriptResources.hasOwnProperty(e))
                  return;
                w = [], d.moduleScriptResources[e] = !t || typeof t.crossOrigin != "string" && typeof t.integrity != "string" ? Ft : [t.crossOrigin, t.integrity], b.preloads.moduleScripts.set(e, w);
                break;
              default:
                if (d.moduleUnknownResources.hasOwnProperty(w)) {
                  var k = d.unknownResources[w];
                  if (k.hasOwnProperty(e)) return;
                } else
                  k = {}, d.moduleUnknownResources[w] = k;
                w = [], k[e] = Ft;
            }
            xr(
              w,
              Pn({ rel: "modulepreload", href: e }, t)
            ), b.bulkPreloads.add(w), _r(c);
          }
        } else Me.m(e, t);
      },
      X: function(e, t) {
        var c = yn || null;
        if (c) {
          var d = c.resumableState, b = c.renderState;
          if (e) {
            var w = d.scriptResources.hasOwnProperty(
              e
            ) ? d.scriptResources[e] : void 0;
            w !== rn && (d.scriptResources[e] = rn, t = Pn({ src: e, async: !0 }, t), w && (w.length === 2 && Ni(t, w), e = b.preloads.scripts.get(e)) && (e.length = 0), e = [], b.scripts.add(e), Pc(e, t), _r(c));
          }
        } else Me.X(e, t);
      },
      S: function(e, t, c) {
        var d = yn || null;
        if (d) {
          var b = d.resumableState, w = d.renderState;
          if (e) {
            t = t || "default";
            var k = w.styles.get(t), A = b.styleResources.hasOwnProperty(e) ? b.styleResources[e] : void 0;
            A !== rn && (b.styleResources[e] = rn, k || (k = {
              precedence: ne(ge(t)),
              rules: [],
              hrefs: [],
              sheets: /* @__PURE__ */ new Map()
            }, w.styles.set(t, k)), t = {
              state: aa,
              props: Pn(
                {
                  rel: "stylesheet",
                  href: e,
                  "data-precedence": t
                },
                c
              )
            }, A && (A.length === 2 && Ni(t.props, A), (w = w.preloads.stylesheets.get(e)) && 0 < w.length ? w.length = 0 : t.state = $c), k.sheets.set(e, t), _r(d));
          }
        } else Me.S(e, t, c);
      },
      M: function(e, t) {
        var c = yn || null;
        if (c) {
          var d = c.resumableState, b = c.renderState;
          if (e) {
            var w = d.moduleScriptResources.hasOwnProperty(e) ? d.moduleScriptResources[e] : void 0;
            w !== rn && (d.moduleScriptResources[e] = rn, t = Pn(
              { src: e, type: "module", async: !0 },
              t
            ), w && (w.length === 2 && Ni(t, w), e = b.preloads.moduleScripts.get(e)) && (e.length = 0), e = [], b.scripts.add(e), Pc(e, t), _r(c));
          }
        } else Me.M(e, t);
      }
    };
    var Hn = 0, nn = 1, nr = 2, sn = 4, Ke = 8, tn = 32, Ae = 64, rn = null, Ft = [];
    Object.freeze(Ft);
    var Be = null;
    N('"></template>');
    var qi = N("<script"), Ei = N("<\/script>"), tr = N('<script src="'), il = N('<script type="module" src="'), al = N(' nonce="'), ol = N(' integrity="'), ql = N(' crossorigin="'), Ri = N(' async=""><\/script>'), wt = N("<style"), Aa = /(<\/|<)(s)(cript)/gi, Ia = N(
      '<script type="importmap">'
    ), lc = N("<\/script>"), Hc = {}, bo = 0, ji = 1, rr = 2, ic = 3, lr = 4, $r = 5, Ci = 6, ac = 7, ki = 8, Uc = 9, ct = N("<!-- -->"), oc = /* @__PURE__ */ new Map(), $i = N(' style="'), yo = N(":"), Ma = N(";"), nt = N(" "), el = N('="'), ze = N('"'), Si = N('=""'), xo = N(
      ge(
        "javascript:throw new Error('React form unexpectedly submitted.')"
      )
    ), ea = N('<input type="hidden"'), Un = N(">"), Cr = N("/>"), na = !1, Oa = !1, cl = !1, Xt = !1, To = !1, ir = !1, Fu = !1, _a = !1, cc = !1, Yc = !1, Gc = !1, jl = N(' selected=""'), Da = N(
      `addEventListener("submit",function(a){if(!a.defaultPrevented){var c=a.target,d=a.submitter,e=c.action,b=d;if(d){var f=d.getAttribute("formAction");null!=f&&(e=f,b=null)}"javascript:throw new Error('React form unexpectedly submitted.')"===e&&(a.preventDefault(),b?(a=document.createElement("input"),a.name=b.name,a.value=b.value,b.parentNode.insertBefore(a,b),b=new FormData(c),a.parentNode.removeChild(a)):b=new FormData(c),a=c.ownerDocument||c,(a.$$reactFormReplay=a.$$reactFormReplay||[]).push(c,d,b))}});`
    ), La = N("<!--F!-->"), ar = N("<!--F-->"), uc = /(<\/|<)(s)(tyle)/gi, sc = N("<!--head-->"), mu = N("<!--body-->"), Au = N("<!--html-->"), Xc = N(`
`), Iu = /^[a-zA-Z][a-zA-Z:_\.\-\d]*$/, Mu = /* @__PURE__ */ new Map(), ut = N("<!DOCTYPE html>"), Zc = /* @__PURE__ */ new Map(), fc = N(
      "requestAnimationFrame(function(){$RT=performance.now()});"
    ), dc = N('<template id="'), Jc = N('"></template>'), Ou = N("<!--&-->"), Vc = N("<!--/&-->"), Qc = N("<!--$-->"), ta = N(
      '<!--$?--><template id="'
    ), ul = N('"></template>'), ts = N("<!--$!-->"), za = N("<!--/$-->"), hc = N("<template"), Ml = N('"'), wo = N(' data-dgst="'), ra = N(' data-msg="'), gc = N(' data-stck="'), la = N(' data-cstck="'), rs = N("></template>"), ls = N('<div hidden id="'), Ol = N('">'), po = N("</div>"), nl = N(
      '<svg aria-hidden="true" style="display:none" id="'
    ), ia = N('">'), Ba = N("</svg>"), sl = N(
      '<math aria-hidden="true" style="display:none" id="'
    ), _u = N('">'), is = N("</math>"), Eo = N('<table hidden id="'), l = N('">'), a = N("</table>"), s = N(
      '<table hidden><tbody id="'
    ), v = N('">'), x = N("</tbody></table>"), E = N('<table hidden><tr id="'), C = N('">'), _ = N("</tr></table>"), m = N(
      '<table hidden><colgroup id="'
    ), D = N('">'), U = N("</colgroup></table>"), Z = N(
      '$RS=function(a,b){a=document.getElementById(a);b=document.getElementById(b);for(a.parentNode.removeChild(a);a.firstChild;)b.parentNode.insertBefore(a.firstChild,b);b.parentNode.removeChild(b)};$RS("'
    ), se = N('$RS("'), ce = N('","'), Ge = N('")<\/script>');
    N('<template data-rsi="" data-sid="'), N('" data-pid="');
    var Se = N(
      `$RB=[];$RV=function(a){$RT=performance.now();for(var b=0;b<a.length;b+=2){var c=a[b],e=a[b+1];null!==e.parentNode&&e.parentNode.removeChild(e);var f=c.parentNode;if(f){var g=c.previousSibling,h=0;do{if(c&&8===c.nodeType){var d=c.data;if("/$"===d||"/&"===d)if(0===h)break;else h--;else"$"!==d&&"$?"!==d&&"$~"!==d&&"$!"!==d&&"&"!==d||h++}d=c.nextSibling;f.removeChild(c);c=d}while(c);for(;e.firstChild;)f.insertBefore(e.firstChild,c);g.data="$";g._reactRetry&&requestAnimationFrame(g._reactRetry)}}a.length=0};
$RC=function(a,b){if(b=document.getElementById(b))(a=document.getElementById(a))?(a.previousSibling.data="$~",$RB.push(a,b),2===$RB.length&&("number"!==typeof $RT?requestAnimationFrame($RV.bind(null,$RB)):(a=performance.now(),setTimeout($RV.bind(null,$RB),2300>a&&2E3<a?2300-a:$RT+300-a)))):b.parentNode.removeChild(b)};`
    );
    ne(
      `$RV=function(A,g){function k(a,b){var e=a.getAttribute(b);e&&(b=a.style,l.push(a,b.viewTransitionName,b.viewTransitionClass),"auto"!==e&&(b.viewTransitionClass=e),(a=a.getAttribute("vt-name"))||(a="_T_"+K++ +"_"),b.viewTransitionName=a,B=!0)}var B=!1,K=0,l=[];try{var f=document.__reactViewTransition;if(f){f.finished.finally($RV.bind(null,g));return}var m=new Map;for(f=1;f<g.length;f+=2)for(var h=g[f].querySelectorAll("[vt-share]"),d=0;d<h.length;d++){var c=h[d];m.set(c.getAttribute("vt-name"),c)}var u=[];for(h=0;h<g.length;h+=2){var C=g[h],x=C.parentNode;if(x){var v=x.getBoundingClientRect();if(v.left||v.top||v.width||v.height){c=C;for(f=0;c;){if(8===c.nodeType){var r=c.data;if("/$"===r)if(0===f)break;else f--;else"$"!==r&&"$?"!==r&&"$~"!==r&&"$!"!==r||f++}else if(1===c.nodeType){d=c;var D=d.getAttribute("vt-name"),y=m.get(D);k(d,y?"vt-share":"vt-exit");y&&(k(y,"vt-share"),m.set(D,null));var E=d.querySelectorAll("[vt-share]");for(d=0;d<E.length;d++){var F=E[d],G=F.getAttribute("vt-name"),
H=m.get(G);H&&(k(F,"vt-share"),k(H,"vt-share"),m.set(G,null))}}c=c.nextSibling}for(var I=g[h+1],t=I.firstElementChild;t;)null!==m.get(t.getAttribute("vt-name"))&&k(t,"vt-enter"),t=t.nextElementSibling;c=x;do for(var n=c.firstElementChild;n;){var J=n.getAttribute("vt-update");J&&"none"!==J&&!l.includes(n)&&k(n,"vt-update");n=n.nextElementSibling}while((c=c.parentNode)&&1===c.nodeType&&"none"!==c.getAttribute("vt-update"));u.push.apply(u,I.querySelectorAll('img[src]:not([loading="lazy"])'))}}}if(B){var z=
document.__reactViewTransition=document.startViewTransition({update:function(){A(g);for(var a=[document.documentElement.clientHeight,document.fonts.ready],b={},e=0;e<u.length;b={g:b.g},e++)if(b.g=u[e],!b.g.complete){var p=b.g.getBoundingClientRect();0<p.bottom&&0<p.right&&p.top<window.innerHeight&&p.left<window.innerWidth&&(p=new Promise(function(w){return function(q){w.g.addEventListener("load",q);w.g.addEventListener("error",q)}}(b)),a.push(p))}return Promise.race([Promise.all(a),new Promise(function(w){var q=
performance.now();setTimeout(w,2300>q&&2E3<q?2300-q:500)})])},types:[]});z.ready.finally(function(){for(var a=l.length-3;0<=a;a-=3){var b=l[a],e=b.style;e.viewTransitionName=l[a+1];e.viewTransitionClass=l[a+1];""===b.getAttribute("style")&&b.removeAttribute("style")}});z.finished.finally(function(){document.__reactViewTransition===z&&(document.__reactViewTransition=null)});$RB=[];return}}catch(a){}A(g)}.bind(null,$RV);`
    );
    var Xe = N('$RC("'), Yn = N(
      `$RM=new Map;$RR=function(n,w,p){function u(q){this._p=null;q()}for(var r=new Map,t=document,h,b,e=t.querySelectorAll("link[data-precedence],style[data-precedence]"),v=[],k=0;b=e[k++];)"not all"===b.getAttribute("media")?v.push(b):("LINK"===b.tagName&&$RM.set(b.getAttribute("href"),b),r.set(b.dataset.precedence,h=b));e=0;b=[];var l,a;for(k=!0;;){if(k){var f=p[e++];if(!f){k=!1;e=0;continue}var c=!1,m=0;var d=f[m++];if(a=$RM.get(d)){var g=a._p;c=!0}else{a=t.createElement("link");a.href=d;a.rel=
"stylesheet";for(a.dataset.precedence=l=f[m++];g=f[m++];)a.setAttribute(g,f[m++]);g=a._p=new Promise(function(q,x){a.onload=u.bind(a,q);a.onerror=u.bind(a,x)});$RM.set(d,a)}d=a.getAttribute("media");!g||d&&!matchMedia(d).matches||b.push(g);if(c)continue}else{a=v[e++];if(!a)break;l=a.getAttribute("data-precedence");a.removeAttribute("media")}c=r.get(l)||h;c===h&&(h=a);r.set(l,a);c?c.parentNode.insertBefore(a,c.nextSibling):(c=t.head,c.insertBefore(a,c.firstChild))}if(p=document.getElementById(n))p.previousSibling.data=
"$~";Promise.all(b).then($RC.bind(null,n,w),$RX.bind(null,n,"CSS failed to load"))};$RR("`
    ), Fn = N('$RR("'), Zt = N('","'), Dr = N('",'), fl = N('"'), Pe = N(")<\/script>");
    N('<template data-rci="" data-bid="'), N('<template data-rri="" data-bid="'), N('" data-sid="'), N('" data-sty="');
    var or = N(
      '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};'
    ), tt = N(
      '$RX=function(b,c,d,e,f){var a=document.getElementById(b);a&&(b=a.previousSibling,b.data="$!",a=a.dataset,c&&(a.dgst=c),d&&(a.msg=d),e&&(a.stck=e),f&&(a.cstck=f),b._reactRetry&&b._reactRetry())};;$RX("'
    ), kr = N('$RX("'), Pi = N('"'), Gn = N(","), Na = N(")<\/script>");
    N('<template data-rxi="" data-bid="'), N('" data-dgst="'), N('" data-msg="'), N('" data-stck="'), N('" data-cstck="');
    var dl = /[<\u2028\u2029]/g, cr = /[&><\u2028\u2029]/g, _l = N(
      ' media="not all" data-precedence="'
    ), ur = N('" data-href="'), Wa = N('">'), Ro = N("</style>"), Sr = !1, Co = !0, sr = [], Ha = N(' data-precedence="'), Du = N('" data-href="'), Dn = N(" "), ko = N('">'), Dl = N("</style>");
    N('<link rel="expect" href="#'), N('" blocking="render"/>');
    var Kc = N(' id="'), Ua = N("["), qc = N(",["), jc = N(","), So = N("]"), aa = 0, $c = 1, $l = 2, Ya = 3, Ga = /[<>\r\n]/g, as = /["';,\r\n]/g, hl = Function.prototype.bind, os = Symbol.for("react.client.reference"), tl = {};
    Object.freeze(tl);
    var oa = {}, Fi = null, eu = {}, nu = {}, tu = /* @__PURE__ */ new Set(), Xa = /* @__PURE__ */ new Set(), cs = /* @__PURE__ */ new Set(), Lu = /* @__PURE__ */ new Set(), Po = /* @__PURE__ */ new Set(), us = /* @__PURE__ */ new Set(), ru = /* @__PURE__ */ new Set(), xs = /* @__PURE__ */ new Set(), zu = /* @__PURE__ */ new Set(), Ts = {
      enqueueSetState: function(e, t, c) {
        var d = e._reactInternals;
        d.queue === null ? Ut(e, "setState") : (d.queue.push(t), c != null && Cl(c));
      },
      enqueueReplaceState: function(e, t, c) {
        e = e._reactInternals, e.replace = !0, e.queue = [t], c != null && Cl(c);
      },
      enqueueForceUpdate: function(e, t) {
        e._reactInternals.queue === null ? Ut(e, "forceUpdate") : t != null && Cl(t);
      }
    }, lu = { id: 1, overflow: "" }, Fo = Math.clz32 ? Math.clz32 : mc, Ss = Math.log, Ps = Math.LN2, ei = Error(
      "Suspense Exception: This is not a real error! It's an implementation detail of `use` to interrupt the current render. You must either rethrow it immediately, or move the `use` call outside of the `try/catch` block. Capturing without rethrowing will lead to unexpected behavior.\n\nTo handle async errors, wrap your component in an error boundary, or call the promise's `.catch` method and pass the result to `use`."
    ), ss = null, Fs = typeof Object.is == "function" ? Object.is : Ac, Lt = null, mi = null, fr = null, ca = null, gl = null, fn = null, Lr = !1, Ai = !1, st = 0, vc = 0, iu = -1, fs = 0, mo = null, Za = null, bc = 0, Ll = !1, Ja, yc = {
      readContext: Ku,
      use: function(e) {
        if (e !== null && typeof e == "object") {
          if (typeof e.then == "function")
            return xt(e);
          if (e.$$typeof === On)
            return Ku(e);
        }
        throw Error(
          "An unsupported type was passed to use(): " + String(e)
        );
      },
      useContext: function(e) {
        return Ja = "useContext", In(), e._currentValue;
      },
      useMemo: lo,
      useReducer: ro,
      useRef: function(e) {
        Lt = In(), fn = to();
        var t = fn.memoizedState;
        return t === null ? (e = { current: e }, Object.seal(e), fn.memoizedState = e) : t;
      },
      useState: function(e) {
        return Ja = "useState", ro(qu, e);
      },
      useInsertionEffect: er,
      useLayoutEffect: er,
      useCallback: function(e, t) {
        return lo(function() {
          return e;
        }, t);
      },
      useImperativeHandle: er,
      useEffect: er,
      useDebugValue: er,
      useDeferredValue: function(e, t) {
        return In(), t !== void 0 ? t : e;
      },
      useTransition: function() {
        return In(), [!1, Mc];
      },
      useId: function() {
        var e = mi.treeContext, t = e.overflow;
        e = e.id, e = (e & ~(1 << 32 - Fo(e) - 1)).toString(32) + t;
        var c = n;
        if (c === null)
          throw Error(
            "Invalid hook call. Hooks can only be called inside of the body of a function component."
          );
        return t = st++, e = "_" + c.idPrefix + "R_" + e, 0 < t && (e += "H" + t.toString(32)), e + "_";
      },
      useSyncExternalStore: function(e, t, c) {
        if (c === void 0)
          throw Error(
            "Missing getServerSnapshot, which is required for server-rendered content. Will revert to client rendering."
          );
        return c();
      },
      useOptimistic: function(e) {
        return In(), [e, Xo];
      },
      useActionState: Zo,
      useFormState: Zo,
      useHostTransitionStatus: function() {
        return In(), on;
      },
      useMemoCache: function(e) {
        for (var t = Array(e), c = 0; c < e; c++)
          t[c] = Il;
        return t;
      },
      useCacheRefresh: function() {
        return io;
      },
      useEffectEvent: function() {
        return Gl;
      }
    }, n = null, r = null, u = {
      getCacheForType: function() {
        throw Error("Not implemented.");
      },
      cacheSignal: function() {
        throw Error("Not implemented.");
      },
      getOwner: function() {
        return r === null ? null : r.componentStack;
      }
    }, h = 0, y, p, P, M, K, L, J;
    Jo.__reactDisabledLog = !0;
    var ie, fe, oe = !1, $ = new (typeof WeakMap == "function" ? WeakMap : Map)(), Ne = {
      react_stack_bottom_frame: function(e, t, c) {
        return e(t, c);
      }
    }, Cn = Ne.react_stack_bottom_frame.bind(Ne), Re = {
      react_stack_bottom_frame: function(e) {
        return e.render();
      }
    }, ln = Re.react_stack_bottom_frame.bind(Re), Jt = {
      react_stack_bottom_frame: function(e) {
        var t = e._init;
        return t(e._payload);
      }
    }, Vt = Jt.react_stack_bottom_frame.bind(Jt), cn = 0;
    if (typeof performance == "object" && typeof performance.now == "function")
      var mt = performance, ni = function() {
        return mt.now();
      };
    else {
      var Pr = Date;
      ni = function() {
        return Pr.now();
      };
    }
    var he = 4, vn = 0, bn = 1, wn = 2, Xn = 3, Ve = 4, Oe = 5, dr = 14, yn = null, Qt = {}, Ln = {}, zr = {}, Ao = {}, Br = !1, ti = !1, ri = !1, li = 0, zt = !1;
    Al(), Al(), Ls.prerender = function(e, t) {
      return new Promise(function(c, d) {
        var b = t ? t.onHeaders : void 0, w;
        b && (w = function(z) {
          b(new Headers(z));
        });
        var k = no(
          t ? t.identifierPrefix : void 0,
          t ? t.unstable_externalRuntimeSrc : void 0,
          t ? t.bootstrapScriptContent : void 0,
          t ? t.bootstrapScripts : void 0,
          t ? t.bootstrapModules : void 0
        ), A = pr(
          e,
          k,
          Wl(
            k,
            void 0,
            t ? t.unstable_externalRuntimeSrc : void 0,
            t ? t.importMap : void 0,
            w,
            t ? t.maxHeadersLength : void 0
          ),
          de(t ? t.namespaceURI : void 0),
          t ? t.progressiveChunkSize : void 0,
          t ? t.onError : void 0,
          function() {
            var z = new ReadableStream(
              {
                type: "bytes",
                pull: function(ee) {
                  qr(A, ee);
                },
                cancel: function(ee) {
                  A.destination = null, Tn(A, ee);
                }
              },
              { highWaterMark: 0 }
            );
            z = {
              postponed: Mn(A),
              prelude: z
            }, c(z);
          },
          void 0,
          void 0,
          d,
          t ? t.onPostpone : void 0
        );
        if (t && t.signal) {
          var X = t.signal;
          if (X.aborted) Tn(A, X.reason);
          else {
            var O = function() {
              Tn(A, X.reason), X.removeEventListener("abort", O);
            };
            X.addEventListener("abort", O);
          }
        }
        Kr(A);
      });
    }, Ls.renderToReadableStream = function(e, t) {
      return new Promise(function(c, d) {
        var b, w, k = new Promise(function(te, j) {
          w = te, b = j;
        }), A = t ? t.onHeaders : void 0, X;
        A && (X = function(te) {
          A(new Headers(te));
        });
        var O = no(
          t ? t.identifierPrefix : void 0,
          t ? t.unstable_externalRuntimeSrc : void 0,
          t ? t.bootstrapScriptContent : void 0,
          t ? t.bootstrapScripts : void 0,
          t ? t.bootstrapModules : void 0
        ), z = ao(
          e,
          O,
          Wl(
            O,
            t ? t.nonce : void 0,
            t ? t.unstable_externalRuntimeSrc : void 0,
            t ? t.importMap : void 0,
            X,
            t ? t.maxHeadersLength : void 0
          ),
          de(t ? t.namespaceURI : void 0),
          t ? t.progressiveChunkSize : void 0,
          t ? t.onError : void 0,
          w,
          function() {
            var te = new ReadableStream(
              {
                type: "bytes",
                pull: function(j) {
                  qr(z, j);
                },
                cancel: function(j) {
                  z.destination = null, Tn(z, j);
                }
              },
              { highWaterMark: 0 }
            );
            te.allReady = k, c(te);
          },
          function(te) {
            k.catch(function() {
            }), d(te);
          },
          b,
          t ? t.onPostpone : void 0,
          t ? t.formState : void 0
        );
        if (t && t.signal) {
          var ee = t.signal;
          if (ee.aborted) Tn(z, ee.reason);
          else {
            var re = function() {
              Tn(z, ee.reason), ee.removeEventListener("abort", re);
            };
            ee.addEventListener("abort", re);
          }
        }
        Kr(z);
      });
    }, Ls.resume = function(e, t, c) {
      return new Promise(function(d, b) {
        var w, k, A = new Promise(function(ee, re) {
          k = ee, w = re;
        }), X = St(
          e,
          t,
          Wl(
            t.resumableState,
            c ? c.nonce : void 0,
            void 0,
            void 0,
            void 0,
            void 0
          ),
          c ? c.onError : void 0,
          k,
          function() {
            var ee = new ReadableStream(
              {
                type: "bytes",
                pull: function(re) {
                  qr(X, re);
                },
                cancel: function(re) {
                  X.destination = null, Tn(X, re);
                }
              },
              { highWaterMark: 0 }
            );
            ee.allReady = A, d(ee);
          },
          function(ee) {
            A.catch(function() {
            }), b(ee);
          },
          w,
          c ? c.onPostpone : void 0
        );
        if (c && c.signal) {
          var O = c.signal;
          if (O.aborted) Tn(X, O.reason);
          else {
            var z = function() {
              Tn(X, O.reason), O.removeEventListener("abort", z);
            };
            O.addEventListener("abort", z);
          }
        }
        Kr(X);
      });
    }, Ls.resumeAndPrerender = function(e, t, c) {
      return new Promise(function(d, b) {
        var w = Sn(
          e,
          t,
          Wl(
            t.resumableState,
            void 0,
            void 0,
            void 0,
            void 0,
            void 0
          ),
          c ? c.onError : void 0,
          function() {
            var X = new ReadableStream(
              {
                type: "bytes",
                pull: function(O) {
                  qr(w, O);
                },
                cancel: function(O) {
                  w.destination = null, Tn(w, O);
                }
              },
              { highWaterMark: 0 }
            );
            X = { postponed: Mn(w), prelude: X }, d(X);
          },
          void 0,
          void 0,
          b,
          c ? c.onPostpone : void 0
        );
        if (c && c.signal) {
          var k = c.signal;
          if (k.aborted) Tn(w, k.reason);
          else {
            var A = function() {
              Tn(w, k.reason), k.removeEventListener("abort", A);
            };
            k.addEventListener("abort", A);
          }
        }
        Kr(w);
      });
    }, Ls.version = "19.2.4";
  }()), Ls;
}
var qs, tf;
process.env.NODE_ENV === "production" ? (qs = _f(), tf = Df()) : (qs = Lf(), tf = zf());
qs.version;
var gf = qs.renderToString;
qs.renderToStaticMarkup;
tf.renderToReadableStream;
tf.resume;
var hf = { exports: {} }, Qs = {};
/**
 * @license React
 * react-jsx-runtime.production.js
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
var mf;
function Bf() {
  if (mf) return Qs;
  mf = 1;
  var Te = Symbol.for("react.transitional.element"), Vn = Symbol.for("react.fragment");
  function G(vt, Qn, kn) {
    var Qe = null;
    if (kn !== void 0 && (Qe = "" + kn), Qn.key !== void 0 && (Qe = "" + Qn.key), "key" in Qn) {
      kn = {};
      for (var Kn in Qn)
        Kn !== "key" && (kn[Kn] = Qn[Kn]);
    } else kn = Qn;
    return Qn = kn.ref, {
      $$typeof: Te,
      type: vt,
      key: Qe,
      ref: Qn !== void 0 ? Qn : null,
      props: kn
    };
  }
  return Qs.Fragment = Vn, Qs.jsx = G, Qs.jsxs = G, Qs;
}
var Ks = {};
/**
 * @license React
 * react-jsx-runtime.development.js
 *
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */
var Af;
function Nf() {
  return Af || (Af = 1, process.env.NODE_ENV !== "production" && function() {
    function Te(F) {
      if (F == null) return null;
      if (typeof F == "function")
        return F.$$typeof === kc ? null : F.displayName || F.name || null;
      if (typeof F == "string") return F;
      switch (F) {
        case Zr:
          return "Fragment";
        case _i:
          return "Profiler";
        case zo:
          return "StrictMode";
        case Ee:
          return "Suspense";
        case Ir:
          return "SuspenseList";
        case bt:
          return "Activity";
      }
      if (typeof F == "object")
        switch (typeof F.tag == "number" && console.error(
          "Received an unexpected object in getComponentNameFromType(). This is likely a bug in React. Please file an issue."
        ), F.$$typeof) {
          case N:
            return "Portal";
          case Di:
            return F.displayName || "Context";
          case Bo:
            return (F._context.displayName || "Context") + ".Consumer";
          case Nt:
            var de = F.render;
            return F = F.displayName, F || (F = de.displayName || de.name || "", F = F !== "" ? "ForwardRef(" + F + ")" : "ForwardRef"), F;
          case Rt:
            return de = F.displayName || null, de !== null ? de : Te(F.type) || "Memo";
          case dn:
            de = F._payload, F = F._init;
            try {
              return Te(F(de));
            } catch {
            }
        }
      return null;
    }
    function Vn(F) {
      return "" + F;
    }
    function G(F) {
      try {
        Vn(F);
        var de = !1;
      } catch {
        de = !0;
      }
      if (de) {
        de = console;
        var We = de.error, Ze = typeof Symbol == "function" && Symbol.toStringTag && F[Symbol.toStringTag] || F.constructor.name || "Object";
        return We.call(
          de,
          "The provided key is an unsupported type %s. This value must be coerced to a string before using it here.",
          Ze
        ), Vn(F);
      }
    }
    function vt(F) {
      if (F === Zr) return "<>";
      if (typeof F == "object" && F !== null && F.$$typeof === dn)
        return "<...>";
      try {
        var de = Te(F);
        return de ? "<" + de + ">" : "<...>";
      } catch {
        return "<...>";
      }
    }
    function Qn() {
      var F = eo.A;
      return F === null ? null : F.getOwner();
    }
    function kn() {
      return Error("react-stack-top-frame");
    }
    function Qe(F) {
      if (Li.call(F, "key")) {
        var de = Object.getOwnPropertyDescriptor(F, "key").get;
        if (de && de.isReactWarning) return !1;
      }
      return F.key !== void 0;
    }
    function Kn(F, de) {
      function We() {
        V || (V = !0, console.error(
          "%s: `key` is not a prop. Trying to access it will result in `undefined` being returned. If you need to access the same value within the child component, you should pass it as a different prop. (https://react.dev/link/special-props)",
          de
        ));
      }
      We.isReactWarning = !0, Object.defineProperty(F, "key", {
        get: We,
        configurable: !0
      });
    }
    function Nl() {
      var F = Te(this.type);
      return ue[F] || (ue[F] = !0, console.error(
        "Accessing element.ref was removed in React 19. ref is now a regular prop. It will be removed from the JSX Element type in a future release."
      )), F = this.props.ref, F !== void 0 ? F : null;
    }
    function Et(F, de, We, Ze, He, Ct) {
      var _e = We.ref;
      return F = {
        $$typeof: ne,
        type: F,
        key: de,
        props: We,
        _owner: Ze
      }, (_e !== void 0 ? _e : null) !== null ? Object.defineProperty(F, "ref", {
        enumerable: !1,
        get: Nl
      }) : Object.defineProperty(F, "ref", { enumerable: !1, value: null }), F._store = {}, Object.defineProperty(F._store, "validated", {
        configurable: !1,
        enumerable: !1,
        writable: !0,
        value: 0
      }), Object.defineProperty(F, "_debugInfo", {
        configurable: !1,
        enumerable: !1,
        writable: !0,
        value: null
      }), Object.defineProperty(F, "_debugStack", {
        configurable: !1,
        enumerable: !1,
        writable: !0,
        value: He
      }), Object.defineProperty(F, "_debugTask", {
        configurable: !1,
        enumerable: !1,
        writable: !0,
        value: Ct
      }), Object.freeze && (Object.freeze(F.props), Object.freeze(F)), F;
    }
    function jt(F, de, We, Ze, He, Ct) {
      var _e = de.children;
      if (_e !== void 0)
        if (Ze)
          if (Wt(_e)) {
            for (Ze = 0; Ze < _e.length; Ze++)
              H(_e[Ze]);
            Object.freeze && Object.freeze(_e);
          } else
            console.error(
              "React.jsx: Static children should always be an array. You are likely explicitly calling React.jsxs or React.jsxDEV. Use the Babel transform instead."
            );
        else H(_e);
      if (Li.call(de, "key")) {
        _e = Te(F);
        var pl = Object.keys(de).filter(function(Mt) {
          return Mt !== "key";
        });
        Ze = 0 < pl.length ? "{key: someKey, " + pl.join(": ..., ") + ": ...}" : "{key: someKey}", le[_e + Ze] || (pl = 0 < pl.length ? "{" + pl.join(": ..., ") + ": ...}" : "{}", console.error(
          `A props object containing a "key" prop is being spread into JSX:
  let props = %s;
  <%s {...props} />
React keys must be passed directly to JSX without using spread:
  let props = %s;
  <%s key={someKey} {...props} />`,
          Ze,
          _e,
          pl,
          _e
        ), le[_e + Ze] = !0);
      }
      if (_e = null, We !== void 0 && (G(We), _e = "" + We), Qe(de) && (G(de.key), _e = "" + de.key), "key" in de) {
        We = {};
        for (var Mr in de)
          Mr !== "key" && (We[Mr] = de[Mr]);
      } else We = de;
      return _e && Kn(
        We,
        typeof F == "function" ? F.displayName || F.name || "Unknown" : F
      ), Et(
        F,
        _e,
        We,
        Qn(),
        He,
        Ct
      );
    }
    function H(F) {
      pe(F) ? F._store && (F._store.validated = 1) : typeof F == "object" && F !== null && F.$$typeof === dn && (F._payload.status === "fulfilled" ? pe(F._payload.value) && F._payload.value._store && (F._payload.value._store.validated = 1) : F._store && (F._store.validated = 1));
    }
    function pe(F) {
      return typeof F == "object" && F !== null && F.$$typeof === ne;
    }
    var Ar = ks, ne = Symbol.for("react.transitional.element"), N = Symbol.for("react.portal"), Zr = Symbol.for("react.fragment"), zo = Symbol.for("react.strict_mode"), _i = Symbol.for("react.profiler"), Bo = Symbol.for("react.consumer"), Di = Symbol.for("react.context"), Nt = Symbol.for("react.forward_ref"), Ee = Symbol.for("react.suspense"), Ir = Symbol.for("react.suspense_list"), Rt = Symbol.for("react.memo"), dn = Symbol.for("react.lazy"), bt = Symbol.for("react.activity"), kc = Symbol.for("react.client.reference"), eo = Ar.__CLIENT_INTERNALS_DO_NOT_USE_OR_WARN_USERS_THEY_CANNOT_UPGRADE, Li = Object.prototype.hasOwnProperty, Wt = Array.isArray, ge = console.createTask ? console.createTask : function() {
      return null;
    };
    Ar = {
      react_stack_bottom_frame: function(F) {
        return F();
      }
    };
    var V, ue = {}, Wl = Ar.react_stack_bottom_frame.bind(
      Ar,
      kn
    )(), no = ge(vt(kn)), le = {};
    Ks.Fragment = Zr, Ks.jsx = function(F, de, We) {
      var Ze = 1e4 > eo.recentlyCreatedOwnerStacks++;
      return jt(
        F,
        de,
        We,
        !1,
        Ze ? Error("react-stack-top-frame") : Wl,
        Ze ? ge(vt(F)) : no
      );
    }, Ks.jsxs = function(F, de, We) {
      var Ze = 1e4 > eo.recentlyCreatedOwnerStacks++;
      return jt(
        F,
        de,
        We,
        !0,
        Ze ? Error("react-stack-top-frame") : Wl,
        Ze ? ge(vt(F)) : no
      );
    };
  }()), Ks;
}
process.env.NODE_ENV === "production" ? hf.exports = Bf() : hf.exports = Nf();
var B = hf.exports;
const Wf = (Te) => Te.md5Hash ? Te.previousMd5 ? Te.previousMd5 === Te.md5Hash ? /* @__PURE__ */ B.jsxs("span", { style: { color: "var(--color-success)", fontWeight: 500 }, children: [
  "✅ ",
  Te.md5Hash,
  " (match)"
] }) : /* @__PURE__ */ B.jsxs(
  "span",
  {
    style: {
      backgroundColor: "var(--color-warning-bg)",
      color: "var(--color-warning)",
      padding: "6px 12px",
      borderRadius: "6px",
      border: "1px solid var(--color-warning)",
      display: "inline-block"
    },
    children: [
      "⚠️ ",
      Te.md5Hash,
      /* @__PURE__ */ B.jsx("br", {}),
      "Previous: ",
      Te.previousMd5
    ]
  }
) : /* @__PURE__ */ B.jsxs("span", { style: { color: "var(--color-warning)", fontWeight: 500 }, children: [
  "➖ ",
  Te.md5Hash,
  " (no baseline)"
] }) : /* @__PURE__ */ B.jsx("span", { style: { color: "var(--color-text-muted)" }, children: "N/A" }), Hf = ({ result: Te }) => {
  const Vn = Te.status === "PASSED", G = Object.entries(Te.environment || {}).map(([vt, Qn]) => `${vt}=${Qn}`).join(", ") || "(none)";
  return /* @__PURE__ */ B.jsxs("div", { className: "card", children: [
    /* @__PURE__ */ B.jsxs(
      "div",
      {
        style: {
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
          marginBottom: "16px",
          paddingBottom: "16px",
          borderBottom: "1px solid var(--color-border)"
        },
        children: [
          /* @__PURE__ */ B.jsxs(
            "span",
            {
              style: {
                fontSize: "1.125rem",
                fontWeight: 600,
                color: "var(--color-text-primary)"
              },
              children: [
                Te.testSuite,
                ".",
                Te.testName
              ]
            }
          ),
          /* @__PURE__ */ B.jsx(
            "span",
            {
              style: {
                padding: "8px 16px",
                borderRadius: "8px",
                fontWeight: 600,
                fontSize: "0.875rem",
                textTransform: "uppercase",
                letterSpacing: "1px",
                ...Vn ? {
                  backgroundColor: "var(--color-success-bg)",
                  color: "var(--color-success)",
                  border: "1px solid var(--color-success)"
                } : {
                  backgroundColor: "var(--color-error-bg)",
                  color: "var(--color-error)",
                  border: "1px solid var(--color-error)"
                }
              },
              children: Te.status
            }
          )
        ]
      }
    ),
    /* @__PURE__ */ B.jsx("table", { className: "detail-table", children: /* @__PURE__ */ B.jsxs("tbody", { children: [
      /* @__PURE__ */ B.jsxs("tr", { children: [
        /* @__PURE__ */ B.jsx("th", { children: "Executable" }),
        /* @__PURE__ */ B.jsx("td", { children: Te.executable })
      ] }),
      /* @__PURE__ */ B.jsxs("tr", { children: [
        /* @__PURE__ */ B.jsx("th", { children: "Test Suite" }),
        /* @__PURE__ */ B.jsx("td", { children: Te.testSuite })
      ] }),
      /* @__PURE__ */ B.jsxs("tr", { children: [
        /* @__PURE__ */ B.jsx("th", { children: "Duration" }),
        /* @__PURE__ */ B.jsxs("td", { children: [
          Te.durationMs,
          "ms"
        ] })
      ] }),
      /* @__PURE__ */ B.jsxs("tr", { children: [
        /* @__PURE__ */ B.jsx("th", { children: "MD5 Hash" }),
        /* @__PURE__ */ B.jsx("td", { children: Wf(Te) })
      ] }),
      /* @__PURE__ */ B.jsxs("tr", { children: [
        /* @__PURE__ */ B.jsx("th", { children: "Environment" }),
        /* @__PURE__ */ B.jsx(
          "td",
          {
            style: {
              fontFamily: "var(--font-mono)",
              fontSize: "0.875rem"
            },
            children: G
          }
        )
      ] })
    ] }) }),
    Te.traceHtmlFilename && /* @__PURE__ */ B.jsx(
      "a",
      {
        href: Te.traceHtmlFilename,
        target: "_blank",
        rel: "noopener noreferrer",
        className: "btn btn-primary",
        style: { marginTop: "16px" },
        children: "📊 View Call Tree"
      }
    ),
    Te.pngFilename ? /* @__PURE__ */ B.jsx("div", { className: "test-image", children: /* @__PURE__ */ B.jsx("img", { src: Te.pngFilename, alt: Te.testName }) }) : /* @__PURE__ */ B.jsx(
      "div",
      {
        className: "test-image",
        style: {
          color: "var(--color-text-muted)",
          fontStyle: "italic"
        },
        children: "No render output (test did not produce image)"
      }
    )
  ] });
}, Uf = ({
  generatedAt: Te,
  tracingEnabled: Vn,
  total: G,
  passed: vt,
  failed: Qn,
  md5Regressions: kn,
  results: Qe,
  updateBaseline: Kn
}) => {
  const Nl = (Et) => new Date(Et).toLocaleString();
  return /* @__PURE__ */ B.jsxs("div", { className: "container", children: [
    /* @__PURE__ */ B.jsx("h1", { children: "Render Test Report" }),
    /* @__PURE__ */ B.jsxs("div", { className: "card", children: [
      /* @__PURE__ */ B.jsxs(
        "div",
        {
          style: {
            display: "flex",
            gap: "24px",
            marginBottom: "16px",
            fontSize: "0.875rem",
            color: "var(--color-text-muted)"
          },
          children: [
            /* @__PURE__ */ B.jsxs("span", { children: [
              "Generated: ",
              Nl(Te)
            ] }),
            /* @__PURE__ */ B.jsxs("span", { children: [
              "Tracing: ",
              Vn ? "Enabled" : "Disabled"
            ] })
          ]
        }
      ),
      Kn && /* @__PURE__ */ B.jsx(
        "div",
        {
          style: {
            marginBottom: "16px",
            padding: "12px 16px",
            backgroundColor: "var(--color-success-bg)",
            color: "var(--color-success)",
            border: "1px solid var(--color-success)",
            borderRadius: "8px",
            fontWeight: 500
          },
          children: "Baseline updated with current MD5 hashes"
        }
      ),
      /* @__PURE__ */ B.jsxs(
        "div",
        {
          style: {
            display: "grid",
            gridTemplateColumns: "repeat(4, 1fr)",
            gap: "24px"
          },
          children: [
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-text-primary)",
                        marginBottom: "8px"
                      },
                      children: G
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Total Tests"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-success)",
                        marginBottom: "8px"
                      },
                      children: vt
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Passed"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-error)",
                        marginBottom: "8px"
                      },
                      children: Qn
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Failed"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-warning)",
                        marginBottom: "8px"
                      },
                      children: kn
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "MD5 Regressions"
                    }
                  )
                ]
              }
            )
          ]
        }
      )
    ] }),
    Qe.map((Et, jt) => /* @__PURE__ */ B.jsx(Hf, { result: Et }, jt))
  ] });
}, Yf = ({
  generatedAt: Te,
  total: Vn,
  passed: G,
  failed: vt,
  results: Qn
}) => {
  const kn = (Qe) => new Date(Qe).toLocaleString();
  return /* @__PURE__ */ B.jsxs("div", { className: "container", children: [
    /* @__PURE__ */ B.jsxs(
      "div",
      {
        style: {
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
          flexWrap: "wrap",
          gap: "12px",
          marginBottom: "8px"
        },
        children: [
          /* @__PURE__ */ B.jsx("h1", { style: { margin: 0 }, children: "Unit Test Report" }),
          /* @__PURE__ */ B.jsx(
            "a",
            {
              href: "render_output/render_report.html",
              style: {
                color: "var(--color-accent-blue)",
                fontSize: "0.9375rem",
                fontWeight: 500
              },
              children: "→ Render tests"
            }
          )
        ]
      }
    ),
    /* @__PURE__ */ B.jsxs("div", { className: "card", children: [
      /* @__PURE__ */ B.jsxs(
        "div",
        {
          style: {
            marginBottom: "16px",
            fontSize: "0.875rem",
            color: "var(--color-text-muted)"
          },
          children: [
            "Generated: ",
            kn(Te)
          ]
        }
      ),
      /* @__PURE__ */ B.jsxs(
        "div",
        {
          style: {
            display: "grid",
            gridTemplateColumns: "repeat(3, 1fr)",
            gap: "24px"
          },
          children: [
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-text-primary)",
                        marginBottom: "8px"
                      },
                      children: Vn
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Total Tests"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-success)",
                        marginBottom: "8px"
                      },
                      children: G
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Passed"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2.5rem",
                        fontWeight: 700,
                        color: "var(--color-error)",
                        marginBottom: "8px"
                      },
                      children: vt
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Failed"
                    }
                  )
                ]
              }
            )
          ]
        }
      )
    ] }),
    /* @__PURE__ */ B.jsxs("div", { className: "card", children: [
      /* @__PURE__ */ B.jsx(
        "h2",
        {
          style: {
            fontSize: "1.25rem",
            marginBottom: "16px",
            color: "var(--color-text-primary)"
          },
          children: "Test Results"
        }
      ),
      /* @__PURE__ */ B.jsxs(
        "table",
        {
          className: "detail-table",
          style: {
            width: "100%",
            borderCollapse: "collapse"
          },
          children: [
            /* @__PURE__ */ B.jsx("thead", { children: /* @__PURE__ */ B.jsxs("tr", { children: [
              /* @__PURE__ */ B.jsx(
                "th",
                {
                  style: {
                    color: "var(--color-text-muted)",
                    fontWeight: 500,
                    textTransform: "uppercase",
                    fontSize: "0.75rem",
                    letterSpacing: "1px",
                    padding: "12px 15px",
                    textAlign: "left",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: "Executable"
                }
              ),
              /* @__PURE__ */ B.jsx(
                "th",
                {
                  style: {
                    color: "var(--color-text-muted)",
                    fontWeight: 500,
                    textTransform: "uppercase",
                    fontSize: "0.75rem",
                    letterSpacing: "1px",
                    padding: "12px 15px",
                    textAlign: "left",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: "Suite"
                }
              ),
              /* @__PURE__ */ B.jsx(
                "th",
                {
                  style: {
                    color: "var(--color-text-muted)",
                    fontWeight: 500,
                    textTransform: "uppercase",
                    fontSize: "0.75rem",
                    letterSpacing: "1px",
                    padding: "12px 15px",
                    textAlign: "left",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: "Test"
                }
              ),
              /* @__PURE__ */ B.jsx(
                "th",
                {
                  style: {
                    color: "var(--color-text-muted)",
                    fontWeight: 500,
                    textTransform: "uppercase",
                    fontSize: "0.75rem",
                    letterSpacing: "1px",
                    padding: "12px 15px",
                    textAlign: "left",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: "Status"
                }
              ),
              /* @__PURE__ */ B.jsx(
                "th",
                {
                  style: {
                    color: "var(--color-text-muted)",
                    fontWeight: 500,
                    textTransform: "uppercase",
                    fontSize: "0.75rem",
                    letterSpacing: "1px",
                    padding: "12px 15px",
                    textAlign: "right",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: "Duration"
                }
              )
            ] }) }),
            /* @__PURE__ */ B.jsx("tbody", { children: Qn.map((Qe, Kn) => /* @__PURE__ */ B.jsxs("tr", { children: [
              /* @__PURE__ */ B.jsx(
                "td",
                {
                  style: {
                    color: "var(--color-text-secondary)",
                    padding: "12px 15px",
                    borderBottom: "1px solid var(--color-border)",
                    fontFamily: "var(--font-mono)",
                    fontSize: "0.875rem"
                  },
                  children: Qe.executable
                }
              ),
              /* @__PURE__ */ B.jsx(
                "td",
                {
                  style: {
                    color: "var(--color-text-secondary)",
                    padding: "12px 15px",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: Qe.testSuite
                }
              ),
              /* @__PURE__ */ B.jsx(
                "td",
                {
                  style: {
                    color: "var(--color-text-secondary)",
                    padding: "12px 15px",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: Qe.testName
                }
              ),
              /* @__PURE__ */ B.jsx(
                "td",
                {
                  style: {
                    padding: "12px 15px",
                    borderBottom: "1px solid var(--color-border)"
                  },
                  children: /* @__PURE__ */ B.jsx(
                    "span",
                    {
                      style: {
                        padding: "4px 10px",
                        borderRadius: "6px",
                        fontWeight: 600,
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        ...Qe.passed ? {
                          backgroundColor: "var(--color-success-bg)",
                          color: "var(--color-success)",
                          border: "1px solid var(--color-success)"
                        } : {
                          backgroundColor: "var(--color-error-bg)",
                          color: "var(--color-error)",
                          border: "1px solid var(--color-error)"
                        }
                      },
                      children: Qe.status
                    }
                  )
                }
              ),
              /* @__PURE__ */ B.jsxs(
                "td",
                {
                  style: {
                    color: "var(--color-text-secondary)",
                    padding: "12px 15px",
                    borderBottom: "1px solid var(--color-border)",
                    textAlign: "right",
                    fontFamily: "var(--font-mono)",
                    fontSize: "0.875rem"
                  },
                  children: [
                    Qe.durationMs,
                    " ms"
                  ]
                }
              )
            ] }, Kn)) })
          ]
        }
      )
    ] })
  ] });
}, Gf = ({
  generatedAt: Te,
  unitTotal: Vn,
  unitPassed: G,
  unitFailed: vt,
  unitResults: Qn,
  renderTotal: kn,
  renderPassed: Qe,
  renderFailed: Kn,
  renderResults: Nl,
  tracingEnabled: Et,
  md5Regressions: jt
}) => {
  const H = (N) => new Date(N).toLocaleString(), pe = Vn + kn, Ar = G + Qe, ne = vt + Kn;
  return /* @__PURE__ */ B.jsxs("div", { className: "container", children: [
    /* @__PURE__ */ B.jsxs(
      "div",
      {
        style: {
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
          flexWrap: "wrap",
          gap: "12px",
          marginBottom: "8px"
        },
        children: [
          /* @__PURE__ */ B.jsx("h1", { style: { margin: 0 }, children: "Test Report" }),
          /* @__PURE__ */ B.jsxs("div", { style: { color: "var(--color-text-muted)", fontSize: "0.875rem" }, children: [
            "Generated: ",
            H(Te)
          ] })
        ]
      }
    ),
    /* @__PURE__ */ B.jsx("div", { className: "card", children: /* @__PURE__ */ B.jsxs(
      "div",
      {
        style: {
          display: "grid",
          gridTemplateColumns: "repeat(4, 1fr)",
          gap: "24px"
        },
        children: [
          /* @__PURE__ */ B.jsxs(
            "div",
            {
              style: {
                padding: "20px",
                backgroundColor: "var(--color-bg-tertiary)",
                borderRadius: "12px",
                border: "1px solid var(--color-border)",
                textAlign: "center"
              },
              children: [
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "2.5rem",
                      fontWeight: 700,
                      color: "var(--color-text-primary)",
                      marginBottom: "8px"
                    },
                    children: pe
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "0.75rem",
                      textTransform: "uppercase",
                      letterSpacing: "1px",
                      color: "var(--color-text-muted)"
                    },
                    children: "Total Tests"
                  }
                )
              ]
            }
          ),
          /* @__PURE__ */ B.jsxs(
            "div",
            {
              style: {
                padding: "20px",
                backgroundColor: "var(--color-bg-tertiary)",
                borderRadius: "12px",
                border: "1px solid var(--color-border)",
                textAlign: "center"
              },
              children: [
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "2.5rem",
                      fontWeight: 700,
                      color: "var(--color-success)",
                      marginBottom: "8px"
                    },
                    children: Ar
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "0.75rem",
                      textTransform: "uppercase",
                      letterSpacing: "1px",
                      color: "var(--color-text-muted)"
                    },
                    children: "Passed"
                  }
                )
              ]
            }
          ),
          /* @__PURE__ */ B.jsxs(
            "div",
            {
              style: {
                padding: "20px",
                backgroundColor: "var(--color-bg-tertiary)",
                borderRadius: "12px",
                border: "1px solid var(--color-border)",
                textAlign: "center"
              },
              children: [
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "2.5rem",
                      fontWeight: 700,
                      color: "var(--color-error)",
                      marginBottom: "8px"
                    },
                    children: ne
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "0.75rem",
                      textTransform: "uppercase",
                      letterSpacing: "1px",
                      color: "var(--color-text-muted)"
                    },
                    children: "Failed"
                  }
                )
              ]
            }
          ),
          /* @__PURE__ */ B.jsxs(
            "div",
            {
              style: {
                padding: "20px",
                backgroundColor: "var(--color-bg-tertiary)",
                borderRadius: "12px",
                border: "1px solid var(--color-border)",
                textAlign: "center"
              },
              children: [
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "2.5rem",
                      fontWeight: 700,
                      color: jt > 0 ? "var(--color-warning)" : "var(--color-success)",
                      marginBottom: "8px"
                    },
                    children: jt
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "div",
                  {
                    style: {
                      fontSize: "0.75rem",
                      textTransform: "uppercase",
                      letterSpacing: "1px",
                      color: "var(--color-text-muted)"
                    },
                    children: "MD5 Regressions"
                  }
                )
              ]
            }
          )
        ]
      }
    ) }),
    /* @__PURE__ */ B.jsxs(
      "div",
      {
        style: {
          display: "flex",
          gap: "8px",
          marginBottom: "20px",
          borderBottom: "2px solid var(--color-border)"
        },
        children: [
          /* @__PURE__ */ B.jsxs(
            "button",
            {
              id: "tab-unit",
              className: "tab-button active",
              "data-tab": "unit",
              style: {
                padding: "12px 24px",
                backgroundColor: "var(--color-bg-secondary)",
                border: "none",
                borderBottom: "2px solid var(--color-accent)",
                color: "var(--color-text-primary)",
                fontSize: "0.9375rem",
                fontWeight: 500,
                cursor: "pointer",
                marginBottom: "-2px",
                transition: "all 0.2s ease"
              },
              children: [
                "Unit Tests (",
                Vn,
                ")"
              ]
            }
          ),
          /* @__PURE__ */ B.jsxs(
            "button",
            {
              id: "tab-render",
              className: "tab-button",
              "data-tab": "render",
              style: {
                padding: "12px 24px",
                backgroundColor: "transparent",
                border: "none",
                borderBottom: "2px solid transparent",
                color: "var(--color-text-muted)",
                fontSize: "0.9375rem",
                fontWeight: 500,
                cursor: "pointer",
                marginBottom: "-2px",
                transition: "all 0.2s ease"
              },
              children: [
                "Render Tests (",
                kn,
                ")",
                Et && /* @__PURE__ */ B.jsx(
                  "span",
                  {
                    style: {
                      marginLeft: "8px",
                      padding: "2px 8px",
                      backgroundColor: "var(--color-accent)",
                      borderRadius: "4px",
                      fontSize: "0.75rem"
                    },
                    children: "Trace"
                  }
                )
              ]
            }
          )
        ]
      }
    ),
    /* @__PURE__ */ B.jsxs("div", { id: "content-unit", className: "tab-content active", children: [
      /* @__PURE__ */ B.jsx("div", { className: "card", style: { marginBottom: "20px" }, children: /* @__PURE__ */ B.jsxs(
        "div",
        {
          style: {
            display: "grid",
            gridTemplateColumns: "repeat(3, 1fr)",
            gap: "24px"
          },
          children: [
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2rem",
                        fontWeight: 700,
                        color: "var(--color-text-primary)",
                        marginBottom: "8px"
                      },
                      children: Vn
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Total"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2rem",
                        fontWeight: 700,
                        color: "var(--color-success)",
                        marginBottom: "8px"
                      },
                      children: G
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Passed"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2rem",
                        fontWeight: 700,
                        color: "var(--color-error)",
                        marginBottom: "8px"
                      },
                      children: vt
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Failed"
                    }
                  )
                ]
              }
            )
          ]
        }
      ) }),
      /* @__PURE__ */ B.jsxs("div", { className: "card", children: [
        /* @__PURE__ */ B.jsx(
          "h2",
          {
            style: {
              fontSize: "1.25rem",
              marginBottom: "16px",
              color: "var(--color-text-primary)"
            },
            children: "Unit Test Results"
          }
        ),
        /* @__PURE__ */ B.jsxs(
          "table",
          {
            className: "detail-table",
            style: {
              width: "100%",
              borderCollapse: "collapse"
            },
            children: [
              /* @__PURE__ */ B.jsx("thead", { children: /* @__PURE__ */ B.jsxs("tr", { children: [
                /* @__PURE__ */ B.jsx(
                  "th",
                  {
                    style: {
                      color: "var(--color-text-muted)",
                      fontWeight: 500,
                      textTransform: "uppercase",
                      fontSize: "0.75rem",
                      letterSpacing: "1px",
                      padding: "12px 15px",
                      textAlign: "left",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: "Executable"
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "th",
                  {
                    style: {
                      color: "var(--color-text-muted)",
                      fontWeight: 500,
                      textTransform: "uppercase",
                      fontSize: "0.75rem",
                      letterSpacing: "1px",
                      padding: "12px 15px",
                      textAlign: "left",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: "Suite"
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "th",
                  {
                    style: {
                      color: "var(--color-text-muted)",
                      fontWeight: 500,
                      textTransform: "uppercase",
                      fontSize: "0.75rem",
                      letterSpacing: "1px",
                      padding: "12px 15px",
                      textAlign: "left",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: "Test"
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "th",
                  {
                    style: {
                      color: "var(--color-text-muted)",
                      fontWeight: 500,
                      textTransform: "uppercase",
                      fontSize: "0.75rem",
                      letterSpacing: "1px",
                      padding: "12px 15px",
                      textAlign: "left",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: "Status"
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "th",
                  {
                    style: {
                      color: "var(--color-text-muted)",
                      fontWeight: 500,
                      textTransform: "uppercase",
                      fontSize: "0.75rem",
                      letterSpacing: "1px",
                      padding: "12px 15px",
                      textAlign: "right",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: "Duration"
                  }
                )
              ] }) }),
              /* @__PURE__ */ B.jsx("tbody", { children: Qn.map((N, Zr) => /* @__PURE__ */ B.jsxs("tr", { children: [
                /* @__PURE__ */ B.jsx(
                  "td",
                  {
                    style: {
                      color: "var(--color-text-secondary)",
                      padding: "12px 15px",
                      borderBottom: "1px solid var(--color-border)",
                      fontFamily: "var(--font-mono)",
                      fontSize: "0.875rem"
                    },
                    children: N.executable
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "td",
                  {
                    style: {
                      color: "var(--color-text-secondary)",
                      padding: "12px 15px",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: N.testSuite
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "td",
                  {
                    style: {
                      color: "var(--color-text-secondary)",
                      padding: "12px 15px",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: N.testName
                  }
                ),
                /* @__PURE__ */ B.jsx(
                  "td",
                  {
                    style: {
                      padding: "12px 15px",
                      borderBottom: "1px solid var(--color-border)"
                    },
                    children: /* @__PURE__ */ B.jsx(
                      "span",
                      {
                        style: {
                          padding: "4px 10px",
                          borderRadius: "6px",
                          fontWeight: 600,
                          fontSize: "0.75rem",
                          textTransform: "uppercase",
                          ...N.passed ? {
                            backgroundColor: "var(--color-success-bg)",
                            color: "var(--color-success)",
                            border: "1px solid var(--color-success)"
                          } : {
                            backgroundColor: "var(--color-error-bg)",
                            color: "var(--color-error)",
                            border: "1px solid var(--color-error)"
                          }
                        },
                        children: N.status
                      }
                    )
                  }
                ),
                /* @__PURE__ */ B.jsxs(
                  "td",
                  {
                    style: {
                      color: "var(--color-text-secondary)",
                      padding: "12px 15px",
                      borderBottom: "1px solid var(--color-border)",
                      textAlign: "right",
                      fontFamily: "var(--font-mono)",
                      fontSize: "0.875rem"
                    },
                    children: [
                      N.durationMs,
                      " ms"
                    ]
                  }
                )
              ] }, Zr)) })
            ]
          }
        )
      ] })
    ] }),
    /* @__PURE__ */ B.jsxs("div", { id: "content-render", className: "tab-content", style: { display: "none" }, children: [
      /* @__PURE__ */ B.jsx("div", { className: "card", style: { marginBottom: "20px" }, children: /* @__PURE__ */ B.jsxs(
        "div",
        {
          style: {
            display: "grid",
            gridTemplateColumns: "repeat(3, 1fr)",
            gap: "24px"
          },
          children: [
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2rem",
                        fontWeight: 700,
                        color: "var(--color-text-primary)",
                        marginBottom: "8px"
                      },
                      children: kn
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Total"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2rem",
                        fontWeight: 700,
                        color: "var(--color-success)",
                        marginBottom: "8px"
                      },
                      children: Qe
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Passed"
                    }
                  )
                ]
              }
            ),
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  padding: "20px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)",
                  textAlign: "center"
                },
                children: [
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "2rem",
                        fontWeight: 700,
                        color: "var(--color-error)",
                        marginBottom: "8px"
                      },
                      children: Kn
                    }
                  ),
                  /* @__PURE__ */ B.jsx(
                    "div",
                    {
                      style: {
                        fontSize: "0.75rem",
                        textTransform: "uppercase",
                        letterSpacing: "1px",
                        color: "var(--color-text-muted)"
                      },
                      children: "Failed"
                    }
                  )
                ]
              }
            )
          ]
        }
      ) }),
      /* @__PURE__ */ B.jsx("div", { style: { display: "flex", flexDirection: "column", gap: "20px" }, children: Nl.map((N, Zr) => /* @__PURE__ */ B.jsxs(
        "div",
        {
          className: "card",
          style: {
            borderLeft: `4px solid ${N.passed ? "var(--color-success)" : "var(--color-error)"}`
          },
          children: [
            /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  display: "flex",
                  justifyContent: "space-between",
                  alignItems: "flex-start",
                  marginBottom: "16px"
                },
                children: [
                  /* @__PURE__ */ B.jsxs("div", { children: [
                    /* @__PURE__ */ B.jsx(
                      "div",
                      {
                        style: {
                          fontFamily: "var(--font-mono)",
                          fontSize: "0.875rem",
                          color: "var(--color-text-muted)",
                          marginBottom: "4px"
                        },
                        children: N.executable
                      }
                    ),
                    /* @__PURE__ */ B.jsxs(
                      "h3",
                      {
                        style: {
                          fontSize: "1.125rem",
                          margin: 0,
                          color: "var(--color-text-primary)"
                        },
                        children: [
                          N.testSuite,
                          ".",
                          N.testName
                        ]
                      }
                    )
                  ] }),
                  /* @__PURE__ */ B.jsxs("div", { style: { display: "flex", gap: "8px", alignItems: "center" }, children: [
                    /* @__PURE__ */ B.jsx(
                      "span",
                      {
                        style: {
                          padding: "4px 12px",
                          borderRadius: "6px",
                          fontWeight: 600,
                          fontSize: "0.75rem",
                          textTransform: "uppercase",
                          ...N.passed ? {
                            backgroundColor: "var(--color-success-bg)",
                            color: "var(--color-success)",
                            border: "1px solid var(--color-success)"
                          } : {
                            backgroundColor: "var(--color-error-bg)",
                            color: "var(--color-error)",
                            border: "1px solid var(--color-error)"
                          }
                        },
                        children: N.status
                      }
                    ),
                    /* @__PURE__ */ B.jsxs(
                      "span",
                      {
                        style: {
                          fontSize: "0.875rem",
                          color: "var(--color-text-muted)",
                          fontFamily: "var(--font-mono)"
                        },
                        children: [
                          N.durationMs,
                          "ms"
                        ]
                      }
                    )
                  ] })
                ]
              }
            ),
            N.md5Hash && /* @__PURE__ */ B.jsxs(
              "div",
              {
                style: {
                  marginBottom: "12px",
                  padding: "8px 12px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "6px",
                  fontFamily: "var(--font-mono)",
                  fontSize: "0.75rem",
                  color: "var(--color-text-muted)"
                },
                children: [
                  "MD5: ",
                  N.md5Hash
                ]
              }
            ),
            N.pngFilename && /* @__PURE__ */ B.jsx(
              "div",
              {
                style: {
                  marginTop: "16px",
                  textAlign: "center",
                  padding: "16px",
                  backgroundColor: "var(--color-bg-tertiary)",
                  borderRadius: "12px",
                  border: "1px solid var(--color-border)"
                },
                children: /* @__PURE__ */ B.jsx(
                  "img",
                  {
                    src: `render_output/${N.pngFilename}`,
                    alt: N.testName,
                    style: {
                      maxWidth: "100%",
                      maxHeight: "400px",
                      border: "1px solid var(--color-border)",
                      borderRadius: "8px"
                    }
                  }
                )
              }
            ),
            N.traceData && /* @__PURE__ */ B.jsx("div", { style: { marginTop: "12px" }, children: /* @__PURE__ */ B.jsx(
              "button",
              {
                className: "trace-btn",
                "data-test-name": `${N.testSuite}.${N.testName}`,
                "data-trace": JSON.stringify(N.traceData),
                style: {
                  backgroundColor: "var(--color-bg-tertiary)",
                  color: "var(--color-accent-blue)",
                  border: "1px solid var(--color-accent-blue)",
                  padding: "6px 14px",
                  borderRadius: "6px",
                  fontSize: "0.875rem",
                  fontWeight: 500,
                  cursor: "pointer"
                },
                children: "Trace"
              }
            ) }),
            N.callgraphs && N.callgraphs.length > 0 && /* @__PURE__ */ B.jsx("div", { style: { marginTop: "12px", display: "flex", gap: "12px" }, children: N.callgraphs.map((zo, _i) => /* @__PURE__ */ B.jsxs(
              "a",
              {
                href: `render_output/${zo}`,
                style: {
                  color: "var(--color-accent-blue)",
                  fontSize: "0.875rem",
                  textDecoration: "none"
                },
                children: [
                  "→ Call Graph ",
                  _i + 1
                ]
              },
              _i
            )) })
          ]
        },
        Zr
      )) })
    ] }),
    /* @__PURE__ */ B.jsxs("div", { id: "trace-modal", className: "modal", style: { display: "none" }, children: [
      /* @__PURE__ */ B.jsx("div", { className: "modal-backdrop", style: {
        position: "fixed",
        top: 0,
        left: 0,
        right: 0,
        bottom: 0,
        backgroundColor: "rgba(0, 0, 0, 0.8)",
        zIndex: 1e3
      } }),
      /* @__PURE__ */ B.jsxs("div", { className: "modal-content", style: {
        position: "fixed",
        top: "50%",
        left: "50%",
        transform: "translate(-50%, -50%)",
        width: "90%",
        maxWidth: "1200px",
        maxHeight: "90vh",
        backgroundColor: "var(--color-bg-secondary)",
        border: "1px solid var(--color-border)",
        borderRadius: "12px",
        zIndex: 1001,
        display: "flex",
        flexDirection: "column"
      }, children: [
        /* @__PURE__ */ B.jsxs("div", { className: "modal-header", style: {
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
          padding: "16px 24px",
          borderBottom: "1px solid var(--color-border)"
        }, children: [
          /* @__PURE__ */ B.jsx("h2", { id: "modal-title", style: { margin: 0, fontSize: "1.25rem" }, children: "Trace" }),
          /* @__PURE__ */ B.jsx(
            "button",
            {
              id: "modal-close",
              style: {
                background: "none",
                border: "none",
                color: "var(--color-text-muted)",
                fontSize: "1.5rem",
                cursor: "pointer",
                padding: "0 4px"
              },
              children: "×"
            }
          )
        ] }),
        /* @__PURE__ */ B.jsxs("div", { className: "modal-body", style: {
          flex: 1,
          overflow: "auto",
          padding: "20px"
        }, children: [
          /* @__PURE__ */ B.jsx("div", { id: "tree-controls", style: { marginBottom: "12px" }, children: /* @__PURE__ */ B.jsx("button", { id: "reset-zoom-btn", style: {
            backgroundColor: "var(--color-bg-tertiary)",
            color: "var(--color-text-primary)",
            border: "1px solid var(--color-border)",
            padding: "6px 14px",
            borderRadius: "6px",
            cursor: "pointer",
            fontSize: "0.875rem"
          }, children: "Reset Zoom" }) }),
          /* @__PURE__ */ B.jsx("div", { id: "modal-tree-container", style: {
            backgroundColor: "var(--color-bg-tertiary)",
            border: "1px solid var(--color-border)",
            borderRadius: "8px",
            minHeight: "500px"
          }, children: /* @__PURE__ */ B.jsx("svg", { id: "modal-tree-svg" }) })
        ] })
      ] })
    ] })
  ] });
}, vf = `
  /* Base reset */
  *, *::before, *::after {
    box-sizing: border-box;
  }

  /* Dark theme CSS variables */
  :root {
    --color-bg-primary: #0f1117;
    --color-bg-secondary: #161b22;
    --color-bg-tertiary: #21262d;
    --color-text-primary: #f1f5f9;
    --color-text-secondary: #cbd5e1;
    --color-text-muted: #94a3b8;
    --color-accent: #8b5cf6;
    --color-accent-hover: #a78bfa;
    --color-accent-blue: #58a6ff;
    --color-accent-blue-hover: #79c0ff;
    --color-border: #2d3748;
    --color-success: #56d364;
    --color-success-bg: rgba(35, 134, 54, 0.2);
    --color-error: #ff7b72;
    --color-error-bg: rgba(218, 54, 51, 0.2);
    --color-warning: #d29922;
    --color-warning-bg: rgba(158, 106, 3, 0.2);
    --shadow-report: 0 10px 40px rgba(0, 0, 0, 0.4);
    --font-sans: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, sans-serif;
    --font-mono: 'Menlo', 'Consolas', 'Monaco', monospace;
  }

  :root {
    color-scheme: dark;
  }

  body {
    font-family: var(--font-sans);
    line-height: 1.6;
    margin: 0;
    padding: 20px;
    background-color: var(--color-bg-primary);
    color: var(--color-text-primary);
  }

  .container {
    max-width: 1200px;
    margin: 0 auto;
  }

  h1 {
    color: var(--color-text-primary);
    border-bottom: 2px solid var(--color-accent);
    padding-bottom: 15px;
    font-size: 2.5em;
    font-weight: 700;
    margin-bottom: 30px;
  }

  a {
    color: var(--color-accent-blue);
    text-decoration: none;
    transition: color 0.2s ease;
  }

  a:hover {
    color: var(--color-accent-blue-hover);
  }

  .card {
    background-color: var(--color-bg-secondary);
    border: 1px solid var(--color-border);
    border-radius: 16px;
    box-shadow: var(--shadow-report);
    padding: 30px;
    margin-bottom: 25px;
  }

  .btn {
    display: inline-block;
    padding: 10px 20px;
    border-radius: 8px;
    font-weight: 500;
    transition: all 0.2s ease;
    cursor: pointer;
    border: none;
    font-size: 1rem;
    text-decoration: none;
  }

  .btn-primary {
    background-color: var(--color-accent);
    color: white;
    box-shadow: 0 4px 12px rgba(139, 92, 246, 0.3);
  }

  .btn-primary:hover {
    background-color: var(--color-accent-hover);
    transform: translateY(-2px);
    box-shadow: 0 6px 20px rgba(139, 92, 246, 0.4);
  }

  .detail-table {
    width: 100%;
    border-collapse: collapse;
    margin-bottom: 20px;
  }

  .detail-table th {
    color: var(--color-text-muted);
    font-weight: 500;
    width: 150px;
    text-transform: uppercase;
    font-size: 0.75rem;
    letter-spacing: 1px;
    padding: 12px 15px;
    text-align: left;
    border-bottom: 1px solid var(--color-border);
  }

  .detail-table td {
    color: var(--color-text-secondary);
    padding: 12px 15px;
    border-bottom: 1px solid var(--color-border);
  }

  .test-image {
    margin-top: 20px;
    text-align: center;
    padding: 20px;
    background-color: var(--color-bg-tertiary);
    border-radius: 12px;
    border: 1px solid var(--color-border);
  }

  .test-image img {
    max-width: 100%;
    border: 1px solid var(--color-border);
    border-radius: 8px;
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
  }
`;
function Jf(Te) {
  const Vn = gf(ks.createElement(Uf, Te));
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Render Test Report</title>
  <style>${vf}</style>
</head>
<body>
  ${Vn}
</body>
</html>`;
}
function Vf(Te, Vn) {
  const G = Te.replace(/<\/script>/g, "<\\/script>");
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Render Trace: ${Vn}</title>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"><\/script>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body { font-family: 'Menlo','Consolas',monospace; background:#0d1117; color:#c9d1d9; padding:20px; }
    h1 { font-size:18px; margin-bottom:4px; color:#58a6ff; }
    .subtitle { font-size:12px; color:#8b949e; margin-bottom:20px; }
    #tree-controls { margin-bottom:12px; }
    button { background:#21262d; color:#c9d1d9; border:1px solid #30363d; padding:6px 14px;
              border-radius:6px; cursor:pointer; font-size:12px; margin-right:8px; }
    button:hover { background:#30363d; }
    #tree-container { background:#161b22; border:1px solid #30363d; border-radius:8px;
                        overflow:auto; padding:20px; min-height:400px; }
    .link { stroke:#30363d; stroke-width:1.5px; }
    .tooltip { position:fixed; background:#1c2128; border:1px solid #30363d; border-radius:6px;
                padding:10px 14px; font-size:11px; pointer-events:none; opacity:0;
                transition:opacity 0.15s; max-width:500px; word-break:break-all;
                z-index:100; color:#c9d1d9; }
    .tooltip .fn  { color:#79c0ff; font-weight:bold; margin-bottom:4px; }
    .tooltip .dur { color:#56d364; }
    .tooltip .addr { color:#8b949e; }
  </style>
</head>
<body>
  <h1>&#9654; Render Trace: ${Vn}</h1>
  <div class="subtitle">Call tree visualization</div>
  <div id="tree-controls">
    <button onclick="resetZoom()">Reset Zoom</button>
  </div>
  <div id="tree-container"><svg id="tree-svg"></svg></div>
  <div class="tooltip" id="tooltip"></div>
<script>
const RAW_TREE = ${G};
const COLORS = ['#58a6ff','#79c0ff','#56d364','#3fb950','#f0883e','#d29922','#bc8cff','#ff7b72'];
const colorFor = depth => COLORS[depth % COLORS.length];
const fmtDur = ns => ns < 1000 ? ns+'ns' : ns < 1e6 ? (ns/1000).toFixed(2)+'us'
                   : ns < 1e9  ? (ns/1e6).toFixed(2)+'ms' : (ns/1e9).toFixed(3)+'s';
const margin = {top:20,right:20,bottom:20,left:20};
const nodeH  = 28;
const NODE_W = 200;
const NODE_H = 80;
let nodeId = 0;

let svgTree, gTree, rootTree, zoomTree;
function initTree() {
  const container = document.getElementById('tree-container');
  const w = Math.max(container.clientWidth - 40, 900);
  svgTree = d3.select('#tree-svg').attr('width', w).attr('height', 600);
  zoomTree = d3.zoom().scaleExtent([0.2,4]).on('zoom', e => gTree.attr('transform', e.transform));
  svgTree.call(zoomTree);
  svgTree.selectAll('defs').remove();
  svgTree.append('defs').append('marker').attr('id','arrowhead').attr('viewBox','0 -5 10 10').attr('refX',10).attr('refY',0).attr('markerWidth',6).attr('markerHeight',6).attr('orient','auto').append('path').attr('d','M0,-5L10,0L0,5').attr('fill','#58a6ff');
  gTree = svgTree.append('g').attr('transform',\`translate(\${margin.left},\${margin.top})\`);
  rootTree = d3.hierarchy(RAW_TREE);
  
  // Calculate initial positions for ALL nodes
  d3.tree().nodeSize([100, 300])(rootTree);
  // Save these as base positions for all nodes
  rootTree.each(d => { d.baseX = d.x; d.baseY = d.y; d.savedX = d.x; d.savedY = d.y; });
  
  rootTree.x0 = rootTree.y0 = 0;
  // Don't collapse anything - show full tree
  updateTree(rootTree);
}

const resetZoom   = () => svgTree.transition().duration(400).call(zoomTree.transform, d3.zoomIdentity.translate(margin.left, margin.top));

function updateLinksTree() {
  const links = rootTree.links();
  const lk = gTree.selectAll('line.link').data(links, d => d.target.id);
  const lkE = lk.enter().insert('line','g').attr('class','link').attr('marker-end','url(#arrowhead)')
    .attr('x1', d => d.source.y + NODE_W/2)
    .attr('y1', d => d.source.x)
    .attr('x2', d => d.source.y + NODE_W/2)
    .attr('y2', d => d.source.x);
  lkE.merge(lk)
    .attr('x1', d => d.source.y + NODE_W/2)
    .attr('y1', d => d.source.x)
    .attr('x2', d => d.target.y - NODE_W/2 + 10)  // Extend line 10px into node so arrow tip lands on edge
    .attr('y2', d => d.target.x);
  lk.exit().remove();
}

function updateTree(src) {
  // Get all currently visible descendants
  const nodes = rootTree.descendants();
  const links = rootTree.links();
  
  // For newly visible nodes (entering), assign positions if they don't have saved positions
  nodes.forEach((d, i) => {
    // If this node doesn't have a saved position yet, use its base position or place relative to parent
    if (d.savedX === undefined || d.savedY === undefined) {
      if (d.parent) {
        // Place relative to parent with offset based on sibling index
        const siblings = d.parent.children || [];
        const idx = siblings.indexOf(d);
        d.savedY = d.parent.savedY + 300;  // 300px to the right of parent
        d.savedX = d.parent.savedX + (idx * 100);  // 100px spacing between siblings
      } else {
        // Root node - use base position
        d.savedX = d.baseX || 0;
        d.savedY = d.baseY || 0;
      }
    }
    // Set current x/y from saved positions
    d.x = d.savedX;
    d.y = d.savedY;
  });
  
  let nc = nodes.length;
  svgTree.attr('height', Math.max(600, nc * nodeH + margin.top + margin.bottom));
  
  const nd = gTree.selectAll('g.node').data(nodes, d => d.id || (d.id = ++nodeId));
  const ndE = nd.enter().append('g').attr('class','node')
    .attr('transform', d => \`translate(\${d.y},\${d.x})\`)
    .on('mouseover', showTooltip).on('mousemove', moveTooltip).on('mouseout', hideTooltip);
  const card = ndE.append('foreignObject').attr('width',NODE_W).attr('height',NODE_H).attr('x',-NODE_W/2).attr('y',-NODE_H/2).append('xhtml:div')
    .style('width', NODE_W+'px').style('height',NODE_H+'px')
    .style('background', d => d.data.endpoint ? '#2d1a0e' : '#161b22')
    .style('border', d => d.data.endpoint ? '2px solid #f0883e' : '2px solid ' + colorFor(d.depth||0))
    .style('border-radius','6px').style('padding','6px 10px').style('font-family','Menlo, Consolas, monospace').style('font-size','11px').style('color','#c9d1d9').style('cursor','pointer')
    .html(d => {
      const fn = d.data.fn||'??'; const title = fn.length>24 ? fn.slice(0,22)+'...' : fn;
      const calls = d.data.calls||1; const total = d.data.total_time_ns != null ? d.data.total_time_ns : (d.data.duration||0);
      const avg = d.data.avg_time_ns != null ? d.data.avg_time_ns : (d.data.duration||0);
      const hiddenChildren = (!d.children && d._children) ? (d.data.child_count||d._children.length||0) : 0;
      return \`<div style="color:\${d.data.endpoint?'#f0883e':'#79c0ff'};font-weight:bold;">\${title}</div><div style="border-top:1px solid #30363d;margin:4px 0;"></div><div style="color:#8b949e;">calls: <span style="color:#c9d1d9;">\${calls}</span> time: <span style="color:#56d364;">\${fmtDur(total)}</span></div>\${hiddenChildren ? '<div style="color:#8b949e;">+'+hiddenChildren+' children</div>' : ''}\`;
    });
  ndE.call(d3.drag()
    .on('start', function(e, d) {
      d3.select(this).raise();
      // Store the grab offset: where on the node we clicked
      d.grabOffsetX = e.x - d.y;
      d.grabOffsetY = e.y - d.x;
    })
    .on('drag', function(e, d) {
      // Apply grab offset so node follows cursor from where we grabbed it
      d.y = e.x - d.grabOffsetX;
      d.x = e.y - d.grabOffsetY;
      d.savedY = d.y;  // Save the new position
      d.savedX = d.x;
      d3.select(this).attr('transform', \`translate(\${d.y},\${d.x})\`);
      updateLinksTree();
    })
    .on('end', function(e, d) {
      // Distinguish click from drag
      const moved = Math.abs(d.x - (e.y - d.grabOffsetY)) + Math.abs(d.y - (e.x - d.grabOffsetX));
      if (moved < 10) {
        // It's a click - expand/collapse
        if (d.children) { d._children = d.children; d.children = null; }
        else if (d._children) { d.children = d._children; d._children = null; }
        updateTree(d);
      }
    }));
  const ndU = ndE.merge(nd);
  ndU.attr('transform', d => \`translate(\${d.y},\${d.x})\`);
  nd.exit().transition().duration(250).remove();
  updateLinksTree();
}

const tipEl = document.getElementById('tooltip');
function showTooltip(ev, d) {
  const data = d.data || d;
  const fn = data.fn || data.label || d.id || '??';
  const dur = data.duration != null ? data.duration : (data.total_time_ns != null ? data.total_time_ns : 0);
  tipEl.innerHTML = \`<div class="fn">\${fn}</div><div class="dur">\${fmtDur(dur)}</div>\`;
  tipEl.style.opacity = '1'; moveTooltip(ev);
}
function moveTooltip(ev) { tipEl.style.left=(ev.clientX+16)+'px'; tipEl.style.top=(ev.clientY+16)+'px'; }
function hideTooltip() { tipEl.style.opacity='0'; }

initTree();
<\/script>
</body>
</html>`;
}
function Qf(Te) {
  const Vn = gf(ks.createElement(Yf, Te));
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Unit Test Report</title>
  <style>${vf}</style>
</head>
<body>
  ${Vn}
</body>
</html>`;
}
function Kf(Te) {
  const Vn = gf(ks.createElement(Gf, Te));
  return `<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Test Report</title>
  <style>${vf}</style>
</head>
<body>
  ${Vn}
  
<script>
(function() {
  function showTab(tabName) {
    // Update buttons
    document.querySelectorAll('.tab-button').forEach(function(btn) {
      if (btn.dataset.tab === tabName) {
        btn.classList.add('active');
        btn.style.backgroundColor = 'var(--color-bg-secondary)';
        btn.style.borderBottom = '2px solid var(--color-accent)';
        btn.style.color = 'var(--color-text-primary)';
      } else {
        btn.classList.remove('active');
        btn.style.backgroundColor = 'transparent';
        btn.style.borderBottom = '2px solid transparent';
        btn.style.color = 'var(--color-text-muted)';
      }
    });
    
    // Update content
    document.querySelectorAll('.tab-content').forEach(function(content) {
      if (content.id === 'content-' + tabName) {
        content.classList.add('active');
        content.style.display = 'block';
      } else {
        content.classList.remove('active');
        content.style.display = 'none';
      }
    });
  }
  
  document.addEventListener('DOMContentLoaded', function() {
    document.querySelectorAll('.tab-button').forEach(function(btn) {
      btn.addEventListener('click', function() {
        showTab(this.dataset.tab);
      });
    });
  });
})();
<\/script>
  
<script src="https://cdnjs.cloudflare.com/ajax/libs/d3/7.8.5/d3.min.js"><\/script>
<script>
(function() {
  const COLORS = ['#58a6ff','#79c0ff','#56d364','#3fb950','#f0883e','#d29922','#bc8cff','#ff7b72'];
  const colorFor = depth => COLORS[depth % COLORS.length];
  const fmtDur = ns => ns < 1000 ? ns+'ns' : ns < 1e6 ? (ns/1000).toFixed(2)+'us' : ns < 1e9 ? (ns/1e6).toFixed(2)+'ms' : (ns/1e9).toFixed(3)+'s';
  const NODE_W = 200;
  const NODE_H = 80;
  let nodeId = 0;
  let currentSvg, currentG, currentRoot, currentZoom;
  let currentTipEl;

  function initTraceModal(traceData, testName) {
    const container = document.getElementById('modal-tree-container');
    const width = container.clientWidth - 40;
    
    // Clear any existing content
    d3.select('#modal-tree-svg').selectAll('*').remove();
    
    currentSvg = d3.select('#modal-tree-svg').attr('width', width).attr('height', 500);
    currentZoom = d3.zoom().scaleExtent([0.2, 4]).on('zoom', e => currentG.attr('transform', e.transform));
    currentSvg.call(currentZoom);
    currentSvg.append('defs').append('marker').attr('id','arrowhead').attr('viewBox','0 -5 10 10').attr('refX',10).attr('refY',0).attr('markerWidth',6).attr('markerHeight',6).attr('orient','auto').append('path').attr('d','M0,-5L10,0L0,5').attr('fill','#58a6ff');
    currentG = currentSvg.append('g').attr('transform', 'translate(40,40)');
    
    // Reset node ID counter for fresh tree
    nodeId = 0;
    
    // Build tree from trace events
    const tree = buildTree(traceData);
    currentRoot = d3.hierarchy(tree);
    
    // Calculate initial positions
    d3.tree().nodeSize([100, 300])(currentRoot);
    currentRoot.each(d => { d.baseX = d.x; d.baseY = d.y; d.savedX = d.x; d.savedY = d.y; });
    currentRoot.x0 = currentRoot.y0 = 0;
    
    // Create tooltip if not exists
    if (!currentTipEl) {
      currentTipEl = document.createElement('div');
      currentTipEl.className = 'tooltip';
      currentTipEl.style.cssText = 'position:fixed;background:#1c2128;border:1px solid #30363d;border-radius:6px;padding:10px 14px;font-size:11px;pointer-events:none;opacity:0;transition:opacity 0.15s;max-width:500px;word-break:break-all;z-index:1002;color:#c9d1d9;';
      document.body.appendChild(currentTipEl);
    }
    
    updateModalTree(currentRoot);
  }
  
  function buildTree(events) {
    // Patterns to filter out (C++ stdlib and testing framework noise)
    const FILTER_PATTERNS = [
      /^testing::/,           // Google Test framework
      /^std::/,              // C++ standard library
      /_Test::_Test\\(/,      // Test class constructors
      /_Test::~_Test\\(/,     // Test class destructors
      /^_Test::/,            // Test class methods
      /\\b__.*\\b/,            // Compiler internals (starts with __)
    ];
    
    function shouldFilter(fn) {
      if (!fn || fn === '??') return false;
      return FILTER_PATTERNS.some(pattern => pattern.test(fn));
    }
    
    const root = {fn: 'root', ts: 0, duration: 0, children: []};
    const stack = [root];
    
    for (const event of events) {
      if (event.type === 'enter') {
        const fn = event.fn || '??';
        
        // Skip filtered functions - don't add to tree
        if (shouldFilter(fn)) {
          // Still push to stack so we can match the exit
          stack.push({filtered: true, fn});
          continue;
        }
        
        const node = {
          fn: fn,
          addr: event.addr || '',
          ts: event.ts_ns,
          duration: 0,
          children: []
        };
        
        // Find the last non-filtered parent
        let parentIdx = stack.length - 1;
        while (parentIdx >= 0 && stack[parentIdx].filtered) {
          parentIdx--;
        }
        
        if (parentIdx >= 0) {
          stack[parentIdx].children.push(node);
        }
        stack.push(node);
      } else if (event.type === 'exit' && stack.length > 1) {
        const node = stack.pop();
        if (!node.filtered) {
          node.duration = event.ts_ns - node.ts;
        }
      }
    }
    
    // Annotate stats
    function annotateStats(node) {
      const children = node.children || [];
      node.child_count = children.length;
      node.calls = node.calls || 1;
      node.total_time_ns = node.duration || 0;
      node.avg_time_ns = node.total_time_ns / (node.calls || 1);
      for (const child of children) annotateStats(child);
    }
    annotateStats(root);
    
    return root;
  }
  
  function updateLinksTree() {
    const links = currentRoot.links();
    const lk = currentG.selectAll('line.link').data(links, d => d.target.id);
    const lkE = lk.enter().insert('line','g').attr('class','link').attr('marker-end','url(#arrowhead)')
      .attr('x1', d => d.source.y + NODE_W/2)
      .attr('y1', d => d.source.x)
      .attr('x2', d => d.source.y + NODE_W/2)
      .attr('y2', d => d.source.x)
      .style('stroke', '#30363d').style('stroke-width', 1.5);
    lkE.merge(lk)
      .attr('x1', d => d.source.y + NODE_W/2)
      .attr('y1', d => d.source.x)
      .attr('x2', d => d.target.y - NODE_W/2 + 10)
      .attr('y2', d => d.target.x);
    lk.exit().remove();
  }
  
  function updateModalTree(src) {
    const nodes = currentRoot.descendants();
    nodes.forEach(d => {
      if (d.savedX === undefined || d.savedY === undefined) {
        if (d.parent) {
          const siblings = d.parent.children || [];
          const idx = siblings.indexOf(d);
          d.savedY = d.parent.savedY + 300;
          d.savedX = d.parent.savedX + (idx * 100);
        } else {
          d.savedX = d.baseX || 0;
          d.savedY = d.baseY || 0;
        }
      }
      d.x = d.savedX;
      d.y = d.savedY;
    });
    
    currentSvg.attr('height', Math.max(500, nodes.length * 28 + 80));
    
    const nd = currentG.selectAll('g.node').data(nodes, d => d.id || (d.id = ++nodeId));
    const ndE = nd.enter().append('g').attr('class','node')
      .attr('transform', d => 'translate(' + d.y + ',' + d.x + ')')
      .on('mouseover', function(e, d) {
        const data = d.data || d;
        const fn = data.fn || '??';
        const dur = data.duration != null ? data.duration : (data.total_time_ns || 0);
        currentTipEl.innerHTML = '<div style="color:#79c0ff;font-weight:bold;">' + fn + '</div><div style="color:#56d364;">' + fmtDur(dur) + '</div>';
        currentTipEl.style.opacity = '1';
        currentTipEl.style.left = (e.clientX + 16) + 'px';
        currentTipEl.style.top = (e.clientY + 16) + 'px';
      })
      .on('mousemove', function(e) {
        currentTipEl.style.left = (e.clientX + 16) + 'px';
        currentTipEl.style.top = (e.clientY + 16) + 'px';
      })
      .on('mouseout', function() {
        currentTipEl.style.opacity = '0';
      });
      
    ndE.append('foreignObject').attr('width',NODE_W).attr('height',NODE_H).attr('x',-NODE_W/2).attr('y',-NODE_H/2).append('xhtml:div')
      .style('width', NODE_W+'px').style('height',NODE_H+'px')
      .style('background', d => d.data.endpoint ? '#2d1a0e' : '#161b22')
      .style('border', d => d.data.endpoint ? '2px solid #f0883e' : '2px solid ' + colorFor(d.depth||0))
      .style('border-radius','6px').style('padding','6px 10px').style('font-family','Menlo, Consolas, monospace').style('font-size','11px').style('color','#c9d1d9').style('cursor','pointer')
      .html(d => {
        const fn = d.data.fn||'??'; const title = fn.length>24 ? fn.slice(0,22)+'...' : fn;
        const calls = d.data.calls||1; const total = d.data.total_time_ns != null ? d.data.total_time_ns : (d.data.duration||0);
        const hiddenChildren = (!d.children && d._children) ? (d.data.child_count||d._children.length||0) : 0;
        return '<div style="color:' + (d.data.endpoint?'#f0883e':'#79c0ff') + ';font-weight:bold;">' + title + '</div><div style="border-top:1px solid #30363d;margin:4px 0;"></div><div style="color:#8b949e;">calls: <span style="color:#c9d1d9;">' + calls + '</span> time: <span style="color:#56d364;">' + fmtDur(total) + '</span></div>' + (hiddenChildren ? '<div style="color:#8b949e;">+'+hiddenChildren+' children</div>' : '');
      });
      
    ndE.call(d3.drag()
      .on('start', function(e, d) {
        d3.select(this).raise();
        d.grabOffsetX = e.x - d.y;
        d.grabOffsetY = e.y - d.x;
      })
      .on('drag', function(e, d) {
        d.y = e.x - d.grabOffsetX;
        d.x = e.y - d.grabOffsetY;
        d.savedY = d.y;
        d.savedX = d.x;
        d3.select(this).attr('transform', 'translate(' + d.y + ',' + d.x + ')');
        updateLinksTree();
      })
      .on('end', function(e, d) {
        const moved = Math.abs(d.x - (e.y - d.grabOffsetY)) + Math.abs(d.y - (e.x - d.grabOffsetX));
        if (moved < 10) {
          if (d.children) { d._children = d.children; d.children = null; }
          else if (d._children) { d.children = d._children; d._children = null; }
          updateModalTree(d);
        }
      }));
      
    ndE.merge(nd).attr('transform', d => 'translate(' + d.y + ',' + d.x + ')');
    nd.exit().remove();
    updateLinksTree();
  }
  
  function resetZoom() {
    if (currentSvg && currentZoom) {
      currentSvg.transition().duration(400).call(currentZoom.transform, d3.zoomIdentity.translate(40, 40));
    }
  }
  
  // Setup event listeners
  document.addEventListener('DOMContentLoaded', function() {
    // Trace button clicks
    document.querySelectorAll('.trace-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        const traceData = JSON.parse(this.dataset.trace.replace(/&quot;/g, '"'));
        const testName = this.dataset.testName;
        document.getElementById('modal-title').textContent = 'Trace: ' + testName;
        document.getElementById('trace-modal').style.display = 'block';
        initTraceModal(traceData, testName);
      });
    });
    
    // Close modal
    document.getElementById('modal-close').addEventListener('click', function() {
      document.getElementById('trace-modal').style.display = 'none';
    });
    
    // Close on backdrop click
    document.querySelector('.modal-backdrop').addEventListener('click', function() {
      document.getElementById('trace-modal').style.display = 'none';
    });
    
    // Reset zoom button
    document.getElementById('reset-zoom-btn').addEventListener('click', resetZoom);
    
    // Escape key to close
    document.addEventListener('keydown', function(e) {
      if (e.key === 'Escape') {
        document.getElementById('trace-modal').style.display = 'none';
      }
    });
  });
})();
<\/script>
</body>
</html>`;
}
export {
  Kf as generateCombinedReportHtml,
  Jf as generateReportHtml,
  Vf as generateTraceHtml,
  Qf as generateUnitReportHtml
};
