(function(t){function z(){for(var a=0;a<g.length;a++)g[a][0](g[a][1]);g=[];m=!1}function n(a,b){g.push([a,b]);m||(m=!0,A(z,0))}function B(a,b){function c(a){p(b,a)}function h(a){k(b,a)}try{a(c,h)}catch(d){h(d)}}function u(a){var b=a.owner,c=b.state_,b=b.data_,h=a[c];a=a.then;if("function"===typeof h){c=l;try{b=h(b)}catch(d){k(a,d)}}v(a,b)||(c===l&&p(a,b),c===q&&k(a,b))}function v(a,b){var c;try{if(a===b)throw new TypeError("A promises callback cannot return that same promise.");if(b&&("function"===
typeof b||"object"===typeof b)){var h=b.then;if("function"===typeof h)return h.call(b,function(d){c||(c=!0,b!==d?p(a,d):w(a,d))},function(b){c||(c=!0,k(a,b))}),!0}}catch(d){return c||k(a,d),!0}return!1}function p(a,b){a!==b&&v(a,b)||w(a,b)}function w(a,b){a.state_===r&&(a.state_=x,a.data_=b,n(C,a))}function k(a,b){a.state_===r&&(a.state_=x,a.data_=b,n(D,a))}function y(a){var b=a.then_;a.then_=void 0;for(a=0;a<b.length;a++)u(b[a])}function C(a){a.state_=l;y(a)}function D(a){a.state_=q;y(a)}function e(a){if("function"!==
typeof a)throw new TypeError("Promise constructor takes a function argument");if(!1===this instanceof e)throw new TypeError("Failed to construct 'Promise': Please use the 'new' operator, this object constructor cannot be called as a function.");this.then_=[];B(a,this)}var f=t.Promise,s=f&&"resolve"in f&&"reject"in f&&"all"in f&&"race"in f&&function(){var a;new f(function(b){a=b});return"function"===typeof a}();"undefined"!==typeof exports&&exports?(exports.Promise=s?f:e,exports.Polyfill=e):"function"==
typeof define&&define.amd?define(function(){return s?f:e}):s||(t.Promise=e);var r="pending",x="sealed",l="fulfilled",q="rejected",E=function(){},A="undefined"!==typeof setImmediate?setImmediate:setTimeout,g=[],m;e.prototype={constructor:e,state_:r,then_:null,data_:void 0,then:function(a,b){var c={owner:this,then:new this.constructor(E),fulfilled:a,rejected:b};this.state_===l||this.state_===q?n(u,c):this.then_.push(c);return c.then},"catch":function(a){return this.then(null,a)}};e.all=function(a){if("[object Array]"!==
Object.prototype.toString.call(a))throw new TypeError("You must pass an array to Promise.all().");return new this(function(b,c){function h(a){e++;return function(c){d[a]=c;--e||b(d)}}for(var d=[],e=0,f=0,g;f<a.length;f++)(g=a[f])&&"function"===typeof g.then?g.then(h(f),c):d[f]=g;e||b(d)})};e.race=function(a){if("[object Array]"!==Object.prototype.toString.call(a))throw new TypeError("You must pass an array to Promise.race().");return new this(function(b,c){for(var e=0,d;e<a.length;e++)(d=a[e])&&"function"===
typeof d.then?d.then(b,c):b(d)})};e.resolve=function(a){return a&&"object"===typeof a&&a.constructor===this?a:new this(function(b){b(a)})};e.reject=function(a){return new this(function(b,c){c(a)})}})("undefined"!=typeof window?window:"undefined"!=typeof global?global:"undefined"!=typeof self?self:this);
/* store.js - Copyright (c) 2010-2017 Marcus Westin */
!function(e){if("object"==typeof exports&&"undefined"!=typeof module)module.exports=e();else if("function"==typeof define&&define.amd)define([],e);else{var t;t="undefined"!=typeof window?window:"undefined"!=typeof global?global:"undefined"!=typeof self?self:this,t.store=e()}}(function(){var define,module,exports;return function e(t,n,r){function o(u,a){if(!n[u]){if(!t[u]){var c="function"==typeof require&&require;if(!a&&c)return c(u,!0);if(i)return i(u,!0);var f=new Error("Cannot find module '"+u+"'");throw f.code="MODULE_NOT_FOUND",f}var s=n[u]={exports:{}};t[u][0].call(s.exports,function(e){var n=t[u][1][e];return o(n?n:e)},s,s.exports,e,t,n,r)}return n[u].exports}for(var i="function"==typeof require&&require,u=0;u<r.length;u++)o(r[u]);return o}({1:[function(e,t,n){"use strict";var r=e("../src/store-engine"),o=e("../storages/all"),i=[e("../plugins/json2")];t.exports=r.createStore(o,i)},{"../plugins/json2":2,"../src/store-engine":4,"../storages/all":6}],2:[function(e,t,n){"use strict";function r(){return e("./lib/json2"),{}}t.exports=r},{"./lib/json2":3}],3:[function(require,module,exports){"use strict";var _typeof="function"==typeof Symbol&&"symbol"==typeof Symbol.iterator?function(e){return typeof e}:function(e){return e&&"function"==typeof Symbol&&e.constructor===Symbol&&e!==Symbol.prototype?"symbol":typeof e};"object"!==("undefined"==typeof JSON?"undefined":_typeof(JSON))&&(JSON={}),function(){function f(e){return e<10?"0"+e:e}function this_value(){return this.valueOf()}function quote(e){return rx_escapable.lastIndex=0,rx_escapable.test(e)?'"'+e.replace(rx_escapable,function(e){var t=meta[e];return"string"==typeof t?t:"\\u"+("0000"+e.charCodeAt(0).toString(16)).slice(-4)})+'"':'"'+e+'"'}function str(e,t){var n,r,o,i,u,a=gap,c=t[e];switch(c&&"object"===("undefined"==typeof c?"undefined":_typeof(c))&&"function"==typeof c.toJSON&&(c=c.toJSON(e)),"function"==typeof rep&&(c=rep.call(t,e,c)),"undefined"==typeof c?"undefined":_typeof(c)){case"string":return quote(c);case"number":return isFinite(c)?String(c):"null";case"boolean":case"null":return String(c);case"object":if(!c)return"null";if(gap+=indent,u=[],"[object Array]"===Object.prototype.toString.apply(c)){for(i=c.length,n=0;n<i;n+=1)u[n]=str(n,c)||"null";return o=0===u.length?"[]":gap?"[\n"+gap+u.join(",\n"+gap)+"\n"+a+"]":"["+u.join(",")+"]",gap=a,o}if(rep&&"object"===("undefined"==typeof rep?"undefined":_typeof(rep)))for(i=rep.length,n=0;n<i;n+=1)"string"==typeof rep[n]&&(r=rep[n],o=str(r,c),o&&u.push(quote(r)+(gap?": ":":")+o));else for(r in c)Object.prototype.hasOwnProperty.call(c,r)&&(o=str(r,c),o&&u.push(quote(r)+(gap?": ":":")+o));return o=0===u.length?"{}":gap?"{\n"+gap+u.join(",\n"+gap)+"\n"+a+"}":"{"+u.join(",")+"}",gap=a,o}}var rx_one=/^[\],:{}\s]*$/,rx_two=/\\(?:["\\\/bfnrt]|u[0-9a-fA-F]{4})/g,rx_three=/"[^"\\\n\r]*"|true|false|null|-?\d+(?:\.\d*)?(?:[eE][+\-]?\d+)?/g,rx_four=/(?:^|:|,)(?:\s*\[)+/g,rx_escapable=/[\\"\u0000-\u001f\u007f-\u009f\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g,rx_dangerous=/[\u0000\u00ad\u0600-\u0604\u070f\u17b4\u17b5\u200c-\u200f\u2028-\u202f\u2060-\u206f\ufeff\ufff0-\uffff]/g;"function"!=typeof Date.prototype.toJSON&&(Date.prototype.toJSON=function(){return isFinite(this.valueOf())?this.getUTCFullYear()+"-"+f(this.getUTCMonth()+1)+"-"+f(this.getUTCDate())+"T"+f(this.getUTCHours())+":"+f(this.getUTCMinutes())+":"+f(this.getUTCSeconds())+"Z":null},Boolean.prototype.toJSON=this_value,Number.prototype.toJSON=this_value,String.prototype.toJSON=this_value);var gap,indent,meta,rep;"function"!=typeof JSON.stringify&&(meta={"\b":"\\b","\t":"\\t","\n":"\\n","\f":"\\f","\r":"\\r",'"':'\\"',"\\":"\\\\"},JSON.stringify=function(e,t,n){var r;if(gap="",indent="","number"==typeof n)for(r=0;r<n;r+=1)indent+=" ";else"string"==typeof n&&(indent=n);if(rep=t,t&&"function"!=typeof t&&("object"!==("undefined"==typeof t?"undefined":_typeof(t))||"number"!=typeof t.length))throw new Error("JSON.stringify");return str("",{"":e})}),"function"!=typeof JSON.parse&&(JSON.parse=function(text,reviver){function walk(e,t){var n,r,o=e[t];if(o&&"object"===("undefined"==typeof o?"undefined":_typeof(o)))for(n in o)Object.prototype.hasOwnProperty.call(o,n)&&(r=walk(o,n),void 0!==r?o[n]=r:delete o[n]);return reviver.call(e,t,o)}var j;if(text=String(text),rx_dangerous.lastIndex=0,rx_dangerous.test(text)&&(text=text.replace(rx_dangerous,function(e){return"\\u"+("0000"+e.charCodeAt(0).toString(16)).slice(-4)})),rx_one.test(text.replace(rx_two,"@").replace(rx_three,"]").replace(rx_four,"")))return j=eval("("+text+")"),"function"==typeof reviver?walk({"":j},""):j;throw new SyntaxError("JSON.parse")})}()},{}],4:[function(e,t,n){"use strict";function r(){var e="undefined"==typeof console?null:console;if(e){var t=e.warn?e.warn:e.log;t.apply(e,arguments)}}function o(e,t,n){n||(n=""),e&&!l(e)&&(e=[e]),t&&!l(t)&&(t=[t]);var o=n?"__storejs_"+n+"_":"",i=n?new RegExp("^"+o):null,v=/^[a-zA-Z0-9_\-]*$/;if(!v.test(n))throw new Error("store.js namespaces can only have alphanumerics + underscores and dashes");var h={_namespacePrefix:o,_namespaceRegexp:i,_testStorage:function(e){try{var t="__storejs__test__";e.write(t,t);var n=e.read(t)===t;return e.remove(t),n}catch(r){return!1}},_assignPluginFnProp:function(e,t){var n=this[t];this[t]=function(){function t(){if(n)return c(arguments,function(e,t){r[t]=e}),n.apply(o,r)}var r=u(arguments,0),o=this,i=[t].concat(r);return e.apply(o,i)}},_serialize:function(e){return JSON.stringify(e)},_deserialize:function(e,t){if(!e)return t;var n="";try{n=JSON.parse(e)}catch(r){n=e}return void 0!==n?n:t},_addStorage:function(e){this.enabled||this._testStorage(e)&&(this.storage=e,this.enabled=!0)},_addPlugin:function(e){var t=this;if(l(e))return void c(e,function(e){t._addPlugin(e)});var n=a(this.plugins,function(t){return e===t});if(!n){if(this.plugins.push(e),!p(e))throw new Error("Plugins must be function values that return objects");var r=e.call(this);if(!d(r))throw new Error("Plugins must return an object of function properties");c(r,function(n,r){if(!p(n))throw new Error("Bad plugin property: "+r+" from plugin "+e.name+". Plugins should only return functions.");t._assignPluginFnProp(n,r)})}},addStorage:function(e){r("store.addStorage(storage) is deprecated. Use createStore([storages])"),this._addStorage(e)}},m=s(h,g,{plugins:[]});return m.raw={},c(m,function(e,t){p(e)&&(m.raw[t]=f(m,e))}),c(e,function(e){m._addStorage(e)}),c(t,function(e){m._addPlugin(e)}),m}var i=e("./util"),u=i.slice,a=i.pluck,c=i.each,f=i.bind,s=i.create,l=i.isList,p=i.isFunction,d=i.isObject;t.exports={createStore:o};var g={version:"2.0.12",enabled:!1,get:function(e,t){var n=this.storage.read(this._namespacePrefix+e);return this._deserialize(n,t)},set:function(e,t){return void 0===t?this.remove(e):(this.storage.write(this._namespacePrefix+e,this._serialize(t)),t)},remove:function(e){this.storage.remove(this._namespacePrefix+e)},each:function(e){var t=this;this.storage.each(function(n,r){e.call(t,t._deserialize(n),(r||"").replace(t._namespaceRegexp,""))})},clearAll:function(){this.storage.clearAll()},hasNamespace:function(e){return this._namespacePrefix=="__storejs_"+e+"_"},createStore:function(){return o.apply(this,arguments)},addPlugin:function(e){this._addPlugin(e)},namespace:function(e){return o(this.storage,this.plugins,e)}}},{"./util":5}],5:[function(e,t,n){(function(e){"use strict";function n(){return Object.assign?Object.assign:function(e,t,n,r){for(var o=1;o<arguments.length;o++)a(Object(arguments[o]),function(t,n){e[n]=t});return e}}function r(){if(Object.create)return function(e,t,n,r){var o=u(arguments,1);return d.apply(this,[Object.create(e)].concat(o))};var e=function(){};return function(t,n,r,o){var i=u(arguments,1);return e.prototype=t,d.apply(this,[new e].concat(i))}}function o(){return String.prototype.trim?function(e){return String.prototype.trim.call(e)}:function(e){return e.replace(/^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g,"")}}function i(e,t){return function(){return t.apply(e,Array.prototype.slice.call(arguments,0))}}function u(e,t){return Array.prototype.slice.call(e,t||0)}function a(e,t){f(e,function(e,n){return t(e,n),!1})}function c(e,t){var n=s(e)?[]:{};return f(e,function(e,r){return n[r]=t(e,r),!1}),n}function f(e,t){if(s(e)){for(var n=0;n<e.length;n++)if(t(e[n],n))return e[n]}else for(var r in e)if(e.hasOwnProperty(r)&&t(e[r],r))return e[r]}function s(e){return null!=e&&"function"!=typeof e&&"number"==typeof e.length}function l(e){return e&&"[object Function]"==={}.toString.call(e)}function p(e){return e&&"[object Object]"==={}.toString.call(e)}var d=n(),g=r(),v=o(),h="undefined"!=typeof window?window:e;t.exports={assign:d,create:g,trim:v,bind:i,slice:u,each:a,map:c,pluck:f,isList:s,isFunction:l,isObject:p,Global:h}}).call(this,"undefined"!=typeof global?global:"undefined"!=typeof self?self:"undefined"!=typeof window?window:{})},{}],6:[function(e,t,n){"use strict";t.exports=[e("./localStorage"),e("./oldFF-globalStorage"),e("./oldIE-userDataStorage"),e("./cookieStorage"),e("./sessionStorage"),e("./memoryStorage")]},{"./cookieStorage":7,"./localStorage":8,"./memoryStorage":9,"./oldFF-globalStorage":10,"./oldIE-userDataStorage":11,"./sessionStorage":12}],7:[function(e,t,n){"use strict";function r(e){if(!e||!c(e))return null;var t="(?:^|.*;\\s*)"+escape(e).replace(/[\-\.\+\*]/g,"\\$&")+"\\s*\\=\\s*((?:[^;](?!;))*[^;]?).*";return unescape(p.cookie.replace(new RegExp(t),"$1"))}function o(e){for(var t=p.cookie.split(/; ?/g),n=t.length-1;n>=0;n--)if(l(t[n])){var r=t[n].split("="),o=unescape(r[0]),i=unescape(r[1]);e(i,o)}}function i(e,t){e&&(p.cookie=escape(e)+"="+escape(t)+"; expires=Tue, 19 Jan 2038 03:14:07 GMT; path=/")}function u(e){e&&c(e)&&(p.cookie=escape(e)+"=; expires=Thu, 01 Jan 1970 00:00:00 GMT; path=/")}function a(){o(function(e,t){u(t)})}function c(e){return new RegExp("(?:^|;\\s*)"+escape(e).replace(/[\-\.\+\*]/g,"\\$&")+"\\s*\\=").test(p.cookie)}var f=e("../src/util"),s=f.Global,l=f.trim;t.exports={name:"cookieStorage",read:r,write:i,each:o,remove:u,clearAll:a};var p=s.document},{"../src/util":5}],8:[function(e,t,n){"use strict";function r(){return s.localStorage}function o(e){return r().getItem(e)}function i(e,t){return r().setItem(e,t)}function u(e){for(var t=r().length-1;t>=0;t--){var n=r().key(t);e(o(n),n)}}function a(e){return r().removeItem(e)}function c(){return r().clear()}var f=e("../src/util"),s=f.Global;t.exports={name:"localStorage",read:o,write:i,each:u,remove:a,clearAll:c}},{"../src/util":5}],9:[function(e,t,n){"use strict";function r(e){return c[e]}function o(e,t){c[e]=t}function i(e){for(var t in c)c.hasOwnProperty(t)&&e(c[t],t)}function u(e){delete c[e]}function a(e){c={}}t.exports={name:"memoryStorage",read:r,write:o,each:i,remove:u,clearAll:a};var c={}},{}],10:[function(e,t,n){"use strict";function r(e){return s[e]}function o(e,t){s[e]=t}function i(e){for(var t=s.length-1;t>=0;t--){var n=s.key(t);e(s[n],n)}}function u(e){return s.removeItem(e)}function a(){i(function(e,t){delete s[e]})}var c=e("../src/util"),f=c.Global;t.exports={name:"oldFF-globalStorage",read:r,write:o,each:i,remove:u,clearAll:a};var s=f.globalStorage},{"../src/util":5}],11:[function(e,t,n){"use strict";function r(e,t){if(!v){var n=c(e);g(function(e){e.setAttribute(n,t),e.save(p)})}}function o(e){if(!v){var t=c(e),n=null;return g(function(e){n=e.getAttribute(t)}),n}}function i(e){g(function(t){for(var n=t.XMLDocument.documentElement.attributes,r=n.length-1;r>=0;r--){var o=n[r];e(t.getAttribute(o.name),o.name)}})}function u(e){var t=c(e);g(function(e){e.removeAttribute(t),e.save(p)})}function a(){g(function(e){var t=e.XMLDocument.documentElement.attributes;e.load(p);for(var n=t.length-1;n>=0;n--)e.removeAttribute(t[n].name);e.save(p)})}function c(e){return e.replace(/^\d/,"___$&").replace(h,"___")}function f(){if(!d||!d.documentElement||!d.documentElement.addBehavior)return null;var e,t,n,r="script";try{t=new ActiveXObject("htmlfile"),t.open(),t.write("<"+r+">document.w=window</"+r+'><iframe src="/favicon.ico"></iframe>'),t.close(),e=t.w.frames[0].document,n=e.createElement("div")}catch(o){n=d.createElement("div"),e=d.body}return function(t){var r=[].slice.call(arguments,0);r.unshift(n),e.appendChild(n),n.addBehavior("#default#userData"),n.load(p),t.apply(this,r),e.removeChild(n)}}var s=e("../src/util"),l=s.Global;t.exports={name:"oldIE-userDataStorage",write:r,read:o,each:i,remove:u,clearAll:a};var p="storejs",d=l.document,g=f(),v=(l.navigator?l.navigator.userAgent:"").match(/ (MSIE 8|MSIE 9|MSIE 10)\./),h=new RegExp("[!\"#$%&'()*+,/\\\\:;<=>?@[\\]^`{|}~]","g")},{"../src/util":5}],12:[function(e,t,n){"use strict";function r(){return s.sessionStorage}function o(e){return r().getItem(e)}function i(e,t){return r().setItem(e,t)}function u(e){for(var t=r().length-1;t>=0;t--){var n=r().key(t);e(o(n),n)}}function a(e){return r().removeItem(e)}function c(){return r().clear()}var f=e("../src/util"),s=f.Global;t.exports={name:"sessionStorage",read:o,write:i,each:u,remove:a,clearAll:c}},{"../src/util":5}]},{},[1])(1)});
function rheaLog(s)
{
	console.log (s);
}

/**************************************************
 * RheaAjaxAnswer
 *
 * oggetto di comodo usato internamente
 */
function RheaAjaxAnswer (requestID)
{
	this.requestID = requestID;
	this.rcv = null;
}


/**************************************************
 * utf8ArrayToStr
 *
 * converte un ArrayBuffer in una stringa
 */
var utf8ArrayToStr = (function () 
{
    var charCache = new Array(128);  // Preallocate the cache for the common single byte chars
    var charFromCodePt = String.fromCodePoint || String.fromCharCode;
    var result = [];

    return function (array) {
        var codePt, byte1;
        var buffLen = array.length;

        result.length = 0;

        for (var i = 0; i < buffLen;) {
            byte1 = array[i++];

            if (byte1 <= 0x7F) {
                codePt = byte1;
            } else if (byte1 <= 0xDF) {
                codePt = ((byte1 & 0x1F) << 6) | (array[i++] & 0x3F);
            } else if (byte1 <= 0xEF) {
                codePt = ((byte1 & 0x0F) << 12) | ((array[i++] & 0x3F) << 6) | (array[i++] & 0x3F);
            } else if (String.fromCodePoint) {
                codePt = ((byte1 & 0x07) << 18) | ((array[i++] & 0x3F) << 12) | ((array[i++] & 0x3F) << 6) | (array[i++] & 0x3F);
            } else {
                codePt = 63;    // Cannot convert four byte code points, so use "?" instead
                i += 3;
            }

            result.push(charCache[codePt] || (charCache[codePt] = charFromCodePt(codePt)));
        }

        return result.join('');
    };
})();




/****************************************************************
 * rheaLoadScript
 *
 *	ritorna una promise
 */
function rheaLoadScript (url)
{
	return new Promise( function(resolve, reject) 
	{
		//console.log ("rheaLoadScript: " +url);
		var script = document.createElement( "script" );
		script.src = url;
		script.onload = resolve;
		script.onerror = reject;
		document.head.appendChild( script );
	});
}

/****************************************************************
 * rheaGetElemByID
 *
 */
function rheaGetElemByID (divID)
{
	return document.getElementById(divID);
}

/****************************************************************
 * rheaSetDivHTMLByName
 *
 */
function rheaSetDivHTMLByName (divID, html)
{
	rheaGetElemByID(divID).innerHTML = html;
}

/****************************************************************
 * rheaGetElemWidth
 *
 *	[elem] deve essere ottenuto da rheaGetElemByID()
 */
function _rheaGetElemProp (p)			{ if (p===undefined) return 0; if (p=="") return 0; return parseInt(p); }
function rheaGetElemWidth (elem)		{ return _rheaGetElemProp(elem.offsetWidth); }
function rheaGetElemHeight (elem)		{ return _rheaGetElemProp(elem.offsetHeight); }
function rheaGetElemLeft (elem)			{ return _rheaGetElemProp(elem.style.left); }
function rheaGetElemTop (elem)			{ return _rheaGetElemProp(elem.style.top); }
function rheaGetElemHTML (elem)			{ return elem.innerHTML; }

function rheaSetElemHTML (elem, html)	{ elem.innerHTML = html; }
function rheaSetElemWidth (elem, pixel)	{ elem.style.width= pixel +"px"; }
function rheaSetElemHeight (elem, pixel){ elem.style.height= pixel +"px"; }
function rheaSetElemLeft (elem, pixel)	{ elem.style.left= pixel +"px"; }
function rheaSetElemTop (elem, pixel)	{ elem.style.top= pixel +"px"; }
function rheaSetDisplayMode (elem, mode) { elem.style.display=mode;}

function rheaSetElemHREF (elem, url)	{ elem.setAttribute("href",url); }

function rheaSetElemBackgroundImage (elem, url) { elem.style.backgroundImage="url(" +url +")"; }
function rheaSetElemBackgroundColor (elem, strColor) { elem.style.backgroundColor=strColor; }
function rheaSetElemTextColor (elem, strColor) { elem.style.color=strColor; }

function rheaAddClassToElem (elem, cssClass)	{ elem.classList.add(cssClass); }
function rheaRemoveClassToElem (elem, cssClass)	{ elem.classList.remove(cssClass); }

function rheaHideElem (elem)	{ rheaSetDisplayMode(elem, "none");}
function rheaShowElem (elem)	{ rheaSetDisplayMode(elem, "block");}



/****************************************************************
 * rheaEasingFn
 *
 */
function rheaEase_linear(t01)		{ return t01; };
function rheaEase_outCubic(t01)		{ return (--t01)*t01*t01+1; }

/****************************************************************
 * rheaSmoothScrollElemLeft
 *
 *	[elem] deve essere ottenuto da rheaGetElemByID()
 *	Scrolla la posizione style.left dal valore attuale al valore [to] nel tempo [durationMSec]
 */
function rheaSmoothScrollElemLeft (elem, to, durationMSec) 
{
    var start = rheaGetElemLeft(elem);
	var change = to - start;
	var currentTimeMSec = 0;
	var stepMSec = 20;
        
    var animateScroll = function()
	{
        currentTimeMSec += stepMSec;
        var t = currentTimeMSec / durationMSec;
		
		if (t >= 1)
		{
			clearInterval(hInterval);
			t = 1;
			rheaSetElemLeft(elem, start+change);
		}
		else
		{
			t = rheaEase_outCubic(t);
			val = start + change*t;
			rheaSetElemLeft(elem,val);
		}
    };
	
	var hInterval = setInterval(animateScroll, stepMSec);
	return hInterval;
}

/****************************************************************
 * rheaSmoothScrollElemTop
 *
 *	[elem] deve essere ottenuto da rheaGetElemByID()
 *	Scrolla la posizione style.top dal valore attuale al valore [to] nel tempo [durationMSec]
 */
function rheaSmoothScrollElemTop (elem, to, durationMSec) 
{
    var start = rheaGetElemTop(elem);
	var change = to - start;
	var currentTimeMSec = 0;
	var stepMSec = 20;
        
    var animateScroll = function()
	{
        currentTimeMSec += stepMSec;
        var t = currentTimeMSec / durationMSec;
		
		if (t >= 1)
		{
			clearInterval(hInterval);
			t = 1;
			rheaSetElemTop(elem, start+change);
		}
		else
		{
			t = rheaEase_outCubic(t);
			val = start + change*t;
			rheaSetElemTop(elem,val);
		}
    };
	
	var hInterval = setInterval(animateScroll, stepMSec);
	return hInterval;
}

/****************************************************************
 * rheaGetURLParamOrDefault
 *
 *	cerca nella queryString il parametro [paramName] e ne ritorna il valore
 *	Se non lo trova, ritorna [defValue]
 */
function rheaGetURLParamOrDefault(paramName, defValue)
{
    var result = null;
    var items = location.search.substr(1).split("&");
    for (var i = 0; i < items.length; i++)
	{
        var tmp = items[i].split("=");
        if (tmp[0] === paramName) 
			return decodeURIComponent(tmp[1]);
    }
    return defValue;
}

/****************************************************************
 * rheaSetDivAsScrollabelByGesture
 *
 *	il div diventa scrollabile su e giu usando i gesti di trascinamento
 */
function ObjRheaScrollDivByGestureInfo(theDiv, contentH, wrapperH) 
{ 
	this.divContent = theDiv; 
	this.mouse_pressed =0; 
	this.mouse_y = 0; 
	this.totalH = rheaGetElemHeight(theDiv);
	this.scroll_miny = -(contentH - wrapperH);
	this.scroll_howMuch = (wrapperH / 2);
	this.scroll_tollerance_at_border = 5;
}
function rheaSetDivAsScrollabelByGesture (divContentID, divWrapperID, contentH, wrapperH)
{
	if (contentH <= wrapperH)
		return;
		
	var theWrapper = rheaGetElemByID(divWrapperID);
	
	//aggiungo freccia su/giu al wrapper
	var arrowDownY = wrapperH - 40;
	var html = "<div id='divArrowUp'   class='bigScrollArrowUp'   style='top:0; display:none'><center><img draggable='false' src='img/big-arrow-up.png' height='30'></center></div>";
	   html += "<div id='divArrowDown' class='bigScrollArrowDown' style='top:" +arrowDownY +"px; display:none'><center><img draggable='false' style='margin-top:11px' src='img/big-arrow-down.png' height='30'></center></div>";
	rheaSetElemHTML(theWrapper,  rheaGetElemHTML(theWrapper) + html);
	rheaShowElem(rheaGetElemByID("divArrowDown"));
	
	var theDiv = rheaGetElemByID(divContentID);
	var info = new ObjRheaScrollDivByGestureInfo(theDiv, contentH, wrapperH);
	theDiv.addEventListener('mousedown', function (ev)
	{
		info.mouse_pressed = 1;
		info.mouse_y = ev.clientY;
	}, true);


	theDiv.addEventListener('mouseup', function (ev) 
	{
		info.mouse_pressed = 0;
	}, true);

	theDiv.addEventListener('mousemove', function (ev) 
	{
		if (!info.mouse_pressed)
			return;
			
		var y = ev.clientY;
		var offset = y - info.mouse_y;
		info.mouse_y = y;
		
		var top = rheaGetElemTop(info.divContent);
		top += offset;
		if (top >= 0)
			top = 0;
		if (top < info.scroll_miny)
			top = info.scroll_miny;
		rheaSetElemTop(info.divContent, top);
		
		if (top < -info.scroll_tollerance_at_border)
			rheaShowElem(rheaGetElemByID("divArrowUp"));
		else
			rheaHideElem(rheaGetElemByID("divArrowUp"));

		if (top < (info.scroll_miny + info.scroll_tollerance_at_border))
			rheaHideElem(rheaGetElemByID("divArrowDown"));
		else
			rheaShowElem(rheaGetElemByID("divArrowDown"));
		
	}, true);
	
	//bindo onclick della freccia giù
	theDiv = rheaGetElemByID("divArrowDown");
	theDiv.addEventListener('click', function (ev) 
	{
		var curY = rheaGetElemTop (info.divContent);
		curY -= info.scroll_howMuch;
		if (curY <= (info.scroll_miny + info.scroll_tollerance_at_border))
		{
			curY = info.scroll_miny;
			rheaHideElem(rheaGetElemByID("divArrowDown"));
		}
		
		rheaShowElem(rheaGetElemByID("divArrowUp"));
		rheaSmoothScrollElemTop(info.divContent, curY, 300);
	}, true)
	
	//bindo onclick della freccia su
	theDiv = rheaGetElemByID("divArrowUp");
	theDiv.addEventListener('click', function (ev) 
	{
		var curY = rheaGetElemTop (info.divContent);
		curY += info.scroll_howMuch;
		if (curY >= -info.scroll_tollerance_at_border)
		{
			rheaHideElem(rheaGetElemByID("divArrowUp"));
			curY = 0;
		}
		
		rheaShowElem(rheaGetElemByID("divArrowDown"));
		rheaSmoothScrollElemTop(info.divContent, curY, 300);
	}, true)		
}//Se si richiede un linguaggio che non è supportato (ie: mancano i file), allora si carica il RHEA_DEFAULT_FALLOFF_LANGUAGE
var 	RHEA_DEFAULT_FALLOFF_LANGUAGE = "GB";

//costanti di comodo da passare come parametro alla fn requestGPUEvent() e/o ricevuti come eventi dalla GPU
var 	RHEA_EVENT_SELECTION_AVAILABILITY_UPDATED = 97;
var	RHEA_EVENT_SELECTION_PRICES_UPDATED = 98;
var	RHEA_EVENT_CREDIT_UPDATED = 99;
var	RHEA_EVENT_CPU_MESSAGE = 100;
var	RHEA_EVENT_SELECTION_REQ_STATUS = 101;
var	RHEA_EVENT_START_SELECTION = 102;
var	RHEA_EVENT_STOP_SELECTION = 103;


/*********************************************************

 * Rhea
 *
 * costruttore
 */
function Rhea()
{
	//lingua corrente
	if (this.Session_getValue ("lang") === undefined)
		this.Session_setValue ("lang", RHEA_DEFAULT_FALLOFF_LANGUAGE);
	
	//ajax    
	this.nextAjaxRequestID = 1;
	this.ajaxReceivedAnswerQ = [];
	var NUM_MAX_CONCURRENT_AJAX_REQUEST = 16;	//num max di richieste "ajax" contemporanee
	for (var i=0; i<NUM_MAX_CONCURRENT_AJAX_REQUEST; i++)
		this.ajaxReceivedAnswerQ[i] = null;
	

	//Selezioni: array di sessione.
	//Alcune informazioni sullo stato delle selezioni sono permanenti, nel senso che sono memorizzate in "session".
	//L'aggiornamento di queste informazioni viene gestito in autonomia e le pagine possono sempre in ogni momento richiedere lo stato delle selezioni
	//utilizzando la fn selection_getBySelNumber()
	//Per sapere quali informazioni sono disponibili per ogni selezione, cerca la fn selection_createEmpty()
	this.nMaxSelection = 48;
	this.selection_sessionRestore();
	
	
	//Variabile di sessione: credito attuale
	//Viene gestita in autonomia, in modo che this.credit rifletta sempre l'attuale credito disponibile
	if (!this.Session_getValue("credit"))
	{
		this.credit = "0";
		this.Session_setValue("credit", "0");
	}
	else
		this.credit = this.Session_getValue("credit");
}

/******************************************************
 * clearSessionData
 * 
 * Utile per pulire le informazioni di sessione. Di solito usata allo startup della GUI
 */
Rhea.prototype.clearSessionData = function ()
{
	this.selection_sessionClear();
	this.Session_setValue ("lang", RHEA_DEFAULT_FALLOFF_LANGUAGE);
	this.credit = "0";
	this.Session_setValue("credit", "0");
}



/******************************************************
 * webSocket_connect
 *
 * 	Ritorna una promise che viene risolta quando la connessione è stabilita.
 *	In caso di errori, viene fatto automaticamente un retry della connessione. 
 *	Continua a fare retry fino a che la connessione è stabilita.
 */
Rhea.prototype.webSocket_connect = function()
{
	var me = this;
	return new Promise( function(resolve, reject) 
	{
		var tryConnect = function()
		{
			rheaLog("Rhea:: trying to connect...");
			me.websocket = new WebSocket("ws://127.0.0.1:2280/", "binary");
			me.websocket.onopen = 		function(evt) 
			{ 
				rheaLog("Rhea::webSocket connected...");
				//me.webSocket_identifyAfterConnection();
				resolve(1); 
			};
			me.websocket.onclose = 		function(evt) { me.webSocket_onClose(evt); };
			me.websocket.onmessage = 	function(evt) { me.webSocket_onRcv(evt) };
			me.websocket.onerror = 		function(evt) { rheaLog("Rhea::onWebSocketErr => ERROR: " + evt.data); setTimeout(tryConnect, 2000); };
		}
		
		setTimeout(tryConnect, 1);
	});
}

Rhea.prototype.webSocket_identifyAfterConnection = function()
{
	rheaLog("Rhea::webSocket identifying..."); 

	var buffer = new Uint8Array(8);
	buffer[0] = 0x01;	//API version
	
	buffer[1] = 0x02;	//identification code MSB
	buffer[2] = 0x03;	//..
	buffer[3] = 0x04;	//..
	buffer[4] = 0x05;	//identification code LSB
	buffer[5] = 0;		//unused
	buffer[6] = 0;		//unused
	buffer[7] = 0;		//unused
	this.sendGPUCommand("W", buffer, 0, 0);
}


/*********************************************************
 * Websocket event handling
 *
 */
Rhea.prototype.webSocket_onClose = function (evt)
{
	rheaLog("Rhea::webSocket_onClose =>  Disconnected");
	this.websocket.close();
}

Rhea.prototype.webSocket_onRcv = function (evt)
{
	//rheaLog("RESPONSE: [len=" +evt.data.size +"] " + evt.data);
	var me = this;
	
	var fileReader = new FileReader();
	fileReader.readAsArrayBuffer(evt.data);
	fileReader.onload = function(event) 
	{
		var data = new Uint8Array(fileReader.result);
		
		//rheaLog ("Rhea::RCV [len=" +data.length +"] [" +utf8ArrayToStr(data) +"]");
		
		//vediamo se è una risposta "ajax" (il buffer deve iniziare con #AJ.ja dove al posto di "." c'è la requestID
		if (data.length > 6)
		{
			if (data[0] == 35 && data[1] == 65 && data[2] == 74 && data[4] == 106 && data[5] == 97)
			{
				var reqID = parseInt(data[3]);
	
				for (var i=0; i<me.ajaxReceivedAnswerQ.length; i++)
				{
					if (me.ajaxReceivedAnswerQ[i] == null)
						continue;
					if (me.ajaxReceivedAnswerQ[i].requestID == reqID)
					{
						me.ajaxReceivedAnswerQ[i].rcv = utf8ArrayToStr(data.subarray(6));
						return;
					}
				}
				return;
			}
		}
		
		//vediamo se è un evento inviato dalla GPU (il buffer deve iniziare con #eVn. dove al posto di "." c'è un byte che indica il tipo di evento
		if (data.length >= 5)
		{
			if (data[0] == 35 && data[1] == 101 && data[2] == 86 && data[3] == 110)
			{
				var eventTypeID = parseInt(data[4]);
				var eventSeqNum = parseInt(data[5]);
				var payloadLen = parseInt(256 * data[6]) + parseInt(data[7]);
				switch (eventTypeID)
				{
				case RHEA_EVENT_SELECTION_AVAILABILITY_UPDATED:
					{
						rheaLog ("RHEA_EVENT_SELECTION_AVAILABILITY_UPDATED:");
						var nSel = parseInt(data[8]);
						for (var i=1; i<=nSel; i++)
							me.selection_getBySelNumber(i).enabled = parseInt(data[8+i]);
						me.selection_sessionStore();
						me.onEvent_selectionAvailabilityUpdated();
					}
					break;
					
				case RHEA_EVENT_SELECTION_PRICES_UPDATED:
					{
						rheaLog ("RHEA_EVENT_SELECTION_PRICES_UPDATED:");
						var prices = utf8ArrayToStr(data.subarray(8)).split("§")
						for (var i=1; i<=prices.length; i++)
							me.selection_getBySelNumber(i).price = prices[i-1];
						me.selection_sessionStore();
						me.onEvent_selectionPricesUpdated();
					}
					break;
					
				case RHEA_EVENT_CREDIT_UPDATED:
					{
						rheaLog ("RHEA_EVENT_CREDIT_UPDATED:");
						me.credit = utf8ArrayToStr(data.subarray(8));
						me.Session_setValue("credit", me.credit);
						me.onEvent_creditUpdated();
					}
					break;
					
				case RHEA_EVENT_CPU_MESSAGE:
					{
						var importanceLevel = data[8];
						var lenInBytes = data[9] * 256 + data[10];
						var msg = utf8ArrayToStr(data.subarray(11));
						rheaLog ("RHEA_EVENT_CPU_MESSAGE: [" +importanceLevel +"] " +msg);
						if (msg != "")
							me.onEvent_cpuMessage(msg, importanceLevel);
					}
					break;					
				
				case RHEA_EVENT_SELECTION_REQ_STATUS:
					{
						var status = parseInt(data[8]);
						rheaLog ("RHEA_EVENT_SELECTION_REQ_STATUS [" +status +"]");
						me.onEvent_selectionReqStatus(status);
					}
					break;
				}
				
				return;
			}
		}

		//è arrivato qualcosa di strano
		rheaLog ("Rhea::RCV [len=" +data.length +"] [" +utf8ArrayToStr(data) +"]");
		
	}
}


/***********************************************************
 * sendBinary
 *
 * invia tramite websocket un buffer di byte.
 * Il parametro [buffer] deve essere di tipo Uint8Array
 */
Rhea.prototype.sendBinary = function (buffer, byteStart, lengthInBytes)
{
	var toSend = new DataView(buffer.buffer, byteStart, lengthInBytes);
	this.websocket.send(toSend);
}

/***********************************************************
 * sendGPUCommand
 *
 *  invia un comando alla GPU.
 *	Ritorna una promise se [bReturnAPromise]==1
 *	Un comando è composto da un commandChar (ovvero un carattere ASCII) e un buffer di byte a seguire
 */
Rhea.prototype.sendGPUCommand = function (commandChar, bufferArrayIN, bReturnAPromise, promiseTimeoutMSec)
{
	var requestID = 255;
	var iReq;
	
	if (bReturnAPromise)
	{
		//requestID deve essere un byte, e deve essere maggiore di 0.
		//Ho messo il limite a 200, ma è un valore arbitrario, non ha un senso specifico. Serve solo ad evitare di eccedere
		//la capienza di un byte. Potevo usare 255 come limite, ma non si sa mai che in futuro si voglia riservare specifici
		//requestID per specifiche richieste
		var requestID = this.nextAjaxRequestID++;
		if (this.nextAjaxRequestID > 200)	
			this.nextAjaxRequestID = 1;

		
		for (iReq=0; iReq<this.ajaxReceivedAnswerQ.length; iReq++)
		{
			if (this.ajaxReceivedAnswerQ[iReq] == null)
			{
				this.ajaxReceivedAnswerQ[iReq] = new RheaAjaxAnswer (requestID);
				break;
			}
		}

		if (iReq >= this.ajaxReceivedAnswerQ.length)
		{
			rheaLog ("Rhea::GPUCommand => too many request");
			return new Promise ( function(resolve, reject) { reject("Rhea::GPUCommand => too many request")} );
		}
	} 	

	//prepara il buffer da inviare
	var bufferLen = 0;
	if (null != bufferArrayIN)
		bufferLen = bufferArrayIN.length;
	var bytesNeeded = 	1 +				//# 
						1 +				//requestID
						1 +				//char del comando
						2 +				//lunghezza di [bufferArrayIN] espressa come byte alto, byte basso
						bufferLen +		//[bufferArrayIN]
						2;				//cheksum espressa come byte alto, byte basso
	
	var buffer = new Uint8Array(bytesNeeded);
		var t = 0;
		buffer[t++] = 35; //'#'
		buffer[t++] = commandChar.charCodeAt(0);
		buffer[t++] = requestID;
		
		buffer[t++] = ((bufferLen & 0xFF00) >> 8);
		buffer[t++] = (bufferLen & 0x00FF);
		for (var i=0; i<bufferLen; i++)
			buffer[t++] = bufferArrayIN[i];

		//checksum
		var ck = 0;
		for (var i=0; i<t; i++)
			ck += buffer[i];

		buffer[t++] = ((ck & 0xFF00) >> 8);
		buffer[t++] = (ck & 0x00FF);

	//invia
	this.sendBinary(buffer, 0, t);
	
	if (bReturnAPromise == 0)
		return;


	var me = this;
	return new Promise( function(resolve, reject) 
					{
						var timeoutMs = promiseTimeoutMSec;
						var check = function()	{
												//console.warn('checking')
												if (me.ajaxReceivedAnswerQ[iReq].rcv != null)
												{
													resolve(me.ajaxReceivedAnswerQ[iReq].rcv);
													me.ajaxReceivedAnswerQ[iReq] = null;
												}
												else if ((timeoutMs -= 100) < 0)
												{
													reject ("timed out [reqID:" +me.ajaxReceivedAnswerQ[iReq].requestID +"][pos:" +iReq +"]");
													me.ajaxReceivedAnswerQ[iReq] = null;
												}
												else
													setTimeout(check, 100);
											}

						setTimeout(check, 100);
					});			
};

/***********************************************************
 * ajax
 *
 * 	Simula il meccanismo di ajax utlizzando il websocket come layer di trasporto.
 *	Ritorna una promise.
 *	Esempio di una richiesta con commandString="time" e vari parametri accessori
		rhea.ajax ("time", 
						{
							"name" : "pippo",
							"number" : i
						})
			.then( function(result) 
			{
				var obj = JSON.parse(result);
				alert (obj.numero);
			})
			.catch( function(result)
			{
				alert ("AJAX::error => " +result);
			});
 */
Rhea.prototype.ajax = function(commandString, plainJSObject)
{
	return this.ajaxWithCustomTimeout(commandString, plainJSObject, 2000);
};
Rhea.prototype.ajaxWithCustomTimeout = function(commandString, plainJSObject, timeoutMSec)
{
	if (plainJSObject=="")
		plainJSObject="{}";
	var jo = JSON.stringify(plainJSObject);

	var bytesNeeded = 	1 							//lenght di [commandString]
						+ commandString.length		//[commandString]
						+ 2							//lenght di [plainJSObject]
						+ jo.length					//[plainJSObject]

	var buffer = new Uint8Array(bytesNeeded);
		var t = 0;
		buffer[t++] = (commandString.length & 0x00FF);
		for (var i=0; i<commandString.length; i++)
			buffer[t++] = commandString.charCodeAt(i);

		buffer[t++] = ((jo.length & 0xFF00) >> 8);
		buffer[t++] = (jo.length & 0x00FF);
		for (var i=0; i<jo.length; i++)
			buffer[t++] = jo.charCodeAt(i);

	return this.sendGPUCommand ("A", buffer, 1, timeoutMSec);
};


/*********************************************************
 * requestGPUEvent
 *
 *	invia una richiesta alla GPU in modo che questa, a sua volta, invii un messaggio
 *	per scatenare l'evento indicato
 *	
 * 	Il parametro eventTypeID può valere una delle const che trovi ad inizio file e il cui nome inizia con RHEA_EVENT_
 */
Rhea.prototype.requestGPUEvent = function (eventTypeID)
{
	var buffer = new Uint8Array(1);
	buffer[0] = eventTypeID;
	this.sendGPUCommand ("E", buffer, 0, 0);
}


/*********************************************************
 * rheaSession
 *
 * Fornisce dei metodi per la memorizzazione/recupero di valori che rimangono
 * persistenti anche cambiando pagina (da una url all'altra).
 * 
 *	Session_setValue(itemName, itemValue)	=>	setta il valore di itemName
 *	Session_getValue(itemName)				=>	ritorna "undefined" oppure il valore di "itemName"
 *
 *	Session_setObject(itemName, plainJSObject)	=>	memorizza un plain object in formato json
 *	Session_getObject(itemName)					=>	ritorna "undefined" oppure il plain object 
 *
 */

/*********************************************************
 * Session_clearValue
 *
 * elimina un entry dal db
 */
Rhea.prototype.Session_clearValue = function (itemName)
{
	store.remove(itemName);
}
/*********************************************************
 * Session_getValue
 *
 */
Rhea.prototype.Session_getValue = function (itemName)
{
	//return localStorage.getItem (itemName);
	return store.get(itemName);
}

Rhea.prototype.Session_getOrDefault = function (itemName, defaultValue)
{
	var r = this.Session_getValue(itemName);
	if (r === undefined)
		return defaultValue;
	return r;
}
/*********************************************************
 * Session_setValue
 *
 */
Rhea.prototype.Session_setValue = function (itemName, itemValue)
{
	//localStorage.setItem (itemName, itemValue);
	store.set(itemName, itemValue);
}

/*********************************************************
 * Session_clearObject
 *
 */
Rhea.prototype.Session_clearObject = function (itemName, plainJSObject)
{
	this.Session_clearValue (itemName);
}


/*********************************************************
 * Session_setObject
 *
 * trasforma [plainJSObject] nella sua rappresentazione JSON e lo memorizza
 * Lavora in coppia con this.Session_getObject 
 */
Rhea.prototype.Session_setObject = function (itemName, plainJSObject)
{
	this.Session_setValue (itemName, JSON.stringify(plainJSObject));
}

/*********************************************************
 * Session_getObject
 *
 * riturna un plain JS object a partire dalla sua descrizione in formato JSON.
 * Lavora in coppia con this.Session_setObject
 */
Rhea.prototype.Session_getObject = function (itemName)
{
	var o = this.Session_getValue (itemName);
	if (!o)
		return o;
	return JSON.parse (o);
}

/*********************************************************
 * selection_createEmpty
 *
 * E' un plain-object con le seguenti prop:
 *
 * 	selNum: 		da 1 a N, indica il numero di selezione cosi' come definito nel da2
 *	enabled:		[0|1]	indica se la selezione è "fattibile", ovvero se non ci sono OFF che ne impediscono l'erogazione
 *	price:			stringa formattata con i decimali e il separatore decimale al punto giusto
 */
Rhea.prototype.selection_createEmpty = function(selNumber)
{
	var s = {
		selNum: 		selNumber,
		enabled: 		0,
		price: 			"0.00"
	};
	return s;
}


/*********************************************************
 * selection_getCount
 *
 *	ritorna il numero di selezioni massime utilizzabili
 */
Rhea.prototype.selection_getCount = function()
{
	return this.nMaxSelection;
}

/*********************************************************
 * selection_getBySelNumber
 *
 *	[selNum]	=> da 1 a this.nMaxSelection
 */
Rhea.prototype.selection_getBySelNumber = function (selNum)
{
	if (selNum < 1 || selNum > this.nMaxSelection)
	{
		rheaLog ("ERR:Rhea.selection_getBySelNumber(" +selNum +") => invalid sel number");
		return this.selection_createEmpty(0);
	}
	
	return this.selectionList[selNum-1];
}

/*********************************************************
 * selection_sessionClear
 *
 *	elimina tutte le info relative allo stato delle selezioni
 */
Rhea.prototype.selection_sessionClear = function()
{
	rheaLog ("Rhea.selection_sessionClear()");
	for (var i=1; i<=this.nMaxSelection; i++)
		this.Session_clearObject("selInfo" +i);
	
	//lo ripopola con i valori di default
	this.selection_sessionRestore();
}

/*********************************************************
 * selection_sessionRestore
 *
 *	leggo tutto dal "db"
 */
Rhea.prototype.selection_sessionRestore = function()
{
	rheaLog ("Rhea.selection_sessionRestore()");
	this.selectionList = [];
	for (var i=1; i<=this.nMaxSelection; i++)
	{
		var s = this.Session_getObject("selInfo" +i);
		if (s === undefined)
		{
			s = this.selection_createEmpty(i);
			this.Session_setObject("selInfo" +i, s);
		}
		this.selectionList.push (s);
	}
}

/*********************************************************
 * selection_sessionStore
 *
 *	salva tutto nel "db"
 */
Rhea.prototype.selection_sessionStore = function()
{
	rheaLog ("Rhea.selection_sessionStore()");
	for (var i=1; i<=this.nMaxSelection; i++)
	{
		this.Session_setObject("selInfo" +i, this.selectionList[i-1]);
	}
}

/*********************************************************
 * selection_start
 *
 *	[iSelNumber] >0 <= [selection_getCount()]
 *	[iSelNumber] deve essere un intero, non un char
 *
 *	Chiede alla GPU di iniziare una selezione.
 *	Per monitorare lo stato della richiesta, utilizzare l'evento rhea.onEvent_selectionReqStatus()
 */
Rhea.prototype.selection_start = function(iSelNumber)
{
	var buffer = new Uint8Array(2);
	buffer[0] = RHEA_EVENT_START_SELECTION;
	buffer[1] = parseInt(iSelNumber);
	this.sendGPUCommand ("E", buffer, 0, 0);
}

/*********************************************************
 * selection_stop
 *
 *	Chiede alla GPU di fermare la selezione corrente
 */
Rhea.prototype.selection_stop = function()
{
	var buffer = new Uint8Array(1);
	buffer[0] = RHEA_EVENT_STOP_SELECTION;
	this.sendGPUCommand ("E", buffer, 0, 0);
}/*********************************************************
 * MMI = Main Menu Icon
 *
 */
var MMI_GRINDER_0	= 0;
var MMI_GRINDER_1	= 1;

var MMI_CUP_SIZE_SMALL = 0;
var MMI_CUP_SIZE_MEDIUM = 1;
var MMI_CUP_SIZE_LARGE = 2;

var MMI_SHOT_TYPE_SINGLE = 0;
var MMI_SHOT_TYPE_DBL = 1;

/*********************************************************
 * MMI_getCount
 *
 *	ritorna il numero di icone da visualizzare nel main menu
 */
Rhea.prototype.MMI_getCount = function()
{
	return rheaMainMenuIcons.length;
}

/*********************************************************
 * MMI_getDisplayName
 *
 *	ritorna la stringa da utilizzare per visualizzare il nome dell'icona.
 *	La stringa è già tradotta nel linguaggio corrente
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_getDisplayName = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	return rheaLang_mainMenuIconName[iIcon];
}

/*********************************************************
 * MMI_getPrice
 *
 *	ritorna una stringa con il prezzo già formattato con il separatore decimale al posto giusto
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_getPrice = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum == 0)
	{
		//Questa è una "virtual selection". Ritorno il prezzo della selezione di default
		var grinder = rheaMainMenuIcons[iIcon].defaultSelection[0];
		var cupSize = rheaMainMenuIcons[iIcon].defaultSelection[1];
		var shotType = rheaMainMenuIcons[iIcon].defaultSelection[2];
		
		selNum = rheaMainMenuIcons[iIcon].linkedSelection[grinder][cupSize][shotType];
	}
	
	return this.selection_getBySelNumber(selNum).price;
}

/*********************************************************
 * MMI_getImgForPageMenu
 *
 *	ritorna il nome della img da usare in pagina menu
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_getImgForPageMenu = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	return "upload/" +rheaMainMenuIcons[iIcon].pageMenuImg;
}


/*********************************************************
 * MMI_getImgForPageConfirm
 *
 *	ritorna il nome della img da usare in pagina confirm
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_getImgForPageConfirm = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	return "upload/" +rheaMainMenuIcons[iIcon].pageConfirmImg;
}


/*********************************************************
 * MMI_getLinkedSelectionNumber
 *
 *	ritorna la selezione [1..48] associata all'icona.
 *	Se ritorna 0, vuol dire che l'icona è associata ad una "selezione virtuale", ovvero una sorta di sottomenu di selezioni
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_getLinkedSelectionNumber = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	return rheaMainMenuIcons[iIcon].selNum;
}

/*********************************************************
 * MMI_canUseSmallCup /MediumCup / LargeCup
 *
 *	ritorna 1 se la selezione associata all'icona può usare la tazza piccola/media/ grande, 0 altrimenti
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_canUseSmallCup = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	if (rheaMainMenuIcons[iIcon].size.charAt(0) == "1")
		return 1;
	return 0;
}

Rhea.prototype.MMI_canUseMediumCup = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	if (rheaMainMenuIcons[iIcon].size.charAt(1) == "1")
		return 1;
	return 0;
}

Rhea.prototype.MMI_canUseLargeCup = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	if (rheaMainMenuIcons[iIcon].size.charAt(2) == "1")
		return 1;
	return 0;
}

/*********************************************************
 * MMI_hasDoubleShot
 *
 *	ritorna 1 se questa icona prevede l'extra shot
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_hasDoubleShot = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return 0;
		
	return rheaMainMenuIcons[iIcon].dblShot;
}

/*********************************************************
 * MMI_hasDblGrinder
 *
 *	ritorna 1 se questa icona prevede la scelta tra grinder 1 e 2
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_hasDblGrinder = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return 0;
		
	return rheaMainMenuIcons[iIcon].grinder2;
}

/*********************************************************
 * MMI_getLinkedSelection
 *
 *	date le 3 opzioni (grinder, cup_size, shot_type) ritorna il numero di selezione da erogare
 *	
 *	[iIcon] va da 0 a MMI_getCount()-1
 *	[GRINDER] = MMI_GRINDER_0 | MMI_GRINDER_1
 *	[CUP_SIZE] = MMI_CUP_SIZE_SMALL | MMI_CUP_SIZE_MEDIUM | MMI_CUP_SIZE_LARGE
 *	[SHOT_TYPE] = MMI_SHOT_TYPE_SINGLE | MMI_SHOT_TYPE_DBL
 */
Rhea.prototype.MMI_getLinkedSelection = function(iIcon, GRINDER, CUP_SIZE, SHOT_TYPE)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;

	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return selNum;
		
	return rheaMainMenuIcons[iIcon].linkedSelection[GRINDER][CUP_SIZE][SHOT_TYPE];
}

/*********************************************************
 * MMI_getDefaultLinkedSelectionOptions
 *
 *	Ritorna un oggetto che descrive le opzioni della selezione di default di una icona "virtuale".
 *	Per opzioni, si intende: quale grinder? quale cupSize? quale shotType?
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 */
Rhea.prototype.MMI_getDefaultLinkedSelectionOptions = function(iIcon)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;
	
	var ret = {
		grinder: MMI_GRINDER_0,
		cupSize: MMI_CUP_SIZE_SMALL,
		shotType: MMI_SHOT_TYPE_SINGLE
	};


	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
	{
		//questo non dovrebbe accadere perchè solo le selezioni "virtuali" hanno opzioni e linkedSelection
		return ret;
	}
	
	//recupero la selezione di default
	ret.grinder = rheaMainMenuIcons[iIcon].defaultSelection[0];
	ret.cupSize = rheaMainMenuIcons[iIcon].defaultSelection[1];
	ret.shotType = rheaMainMenuIcons[iIcon].defaultSelection[2];	
	return ret;
}


/*********************************************************
 * MMI_getLinkedSelectionOptions
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 *	[selNum] va da 1 a 48
 */
Rhea.prototype.MMI_getLinkedSelectionOptions = function(iIcon, selNum)
{
	if (iIcon < 0) iIcon = 0;
	if (iIcon >= this.MMI_getCount()) iIcon = this.MMI_getCount()-1;

	var ret = {
		grinder: MMI_GRINDER_0,
		cupSize: MMI_CUP_SIZE_SMALL,
		shotType: MMI_SHOT_TYPE_SINGLE
	};


	//in teoria non si dovrebbe mai chiamare questa fn su mainIcon che non siano "virtual"
	var selNum = rheaMainMenuIcons[iIcon].selNum;
	if (selNum != 0)
		return ret;
		
	//cerco la [selNum] all'interno di linkedSelection
	for (var grinder=0; grinder<=1; grinder++)
	{
		for (var cupSize=0; cupSize<=2; cupSize++)
		{
			for (var shotType=0; shotType<=1; shotType++)
			{
				if (selNum == rheaMainMenuIcons[iIcon].linkedSelection[grinder][cupSize][shotType])
				{
					ret.grinder = grinder;
					ret.cupSize = cupSize;
					ret.shotType = shotType;
					return ret;
				}
				
			}
		}
	}
	
	return ret;
}

/*********************************************************
 * MMI_isEnabled
 *
 *	[iIcon] va da 0 a MMI_getCount()-1
 *
 *	ritorna 1 se la selezione (o le selezioni) associate all'icona sono tutte abilitate
 */
Rhea.prototype.MMI_isEnabled = function(iIcon)
{
	if (iIcon < 0) return 0;
	if (iIcon >= this.MMI_getCount()) return 0;


	var selNum = this.MMI_getLinkedSelectionNumber(iIcon);
	if (selNum == 0)
	{
		for (var grinder=0; grinder<2; grinder++)
		{
			for (var cup=0; cup<3; cup++)
			{
				for (var shotType=0; shotType<2; shotType++)
				{
					selNum = this.MMI_getLinkedSelection(iIcon, grinder, cup, shotType);			
					if (selNum > 0)
					{
						if (this.selection_getBySelNumber(selNum).enabled == 0)
						{
							//rheaLog("Icon " +iIcon + " disabled due to selNum=" +selNum +" not available");
							return 0;						
						}
					}
				}
				
			}
		}
		return 1;
	}
	else
		return this.selection_getBySelNumber(selNum).enabled;
}/******************************************************************
 * lang_loadLang
 *
 *	Ritorna una promise che viene soddisfatta quando il nuovo linguaggio è stato caricato.
 *
 *	Il parametro [twoCharISOLangCode] deve essere un stringa di 2 caratteri rappresentativa del linguaggio da caricare.
 *	Le traduzioni si trovano nella file config/lang/twoCharISOLangCode/translation.js
 *
 *	Se si richiede un linguaggio non disponibile (ie: il file config/lang/twoCharISOLangCode/translation.js non esiste), allora viene automaticamente
 *	caricato il linguaggio RHEA_DEFAULT_FALLOFF_LANGUAGE, che si suppone esistere sempre
 */
Rhea.prototype.lang_loadLang = function (twoCharISOLangCode)
{
	var me = this;
	
	var url = "config/lang/" +twoCharISOLangCode +"/translation.js";

	//carica lo script js che contiene le traduzioni principali nella lingua selezionata
	return rheaLoadScript(url)
		.then( function(result) 
		{
			return new Promise( function(resolve, reject) 
			{
				rheaLog ("language loaded [" +twoCharISOLangCode +"]=> OK");
				me.Session_setValue ("lang", twoCharISOLangCode);
				resolve(1);
			})
		})
		.catch( function(result)
		{
			rheaLog ("language error [" +twoCharISOLangCode +"]=> " +result);
			return me.lang_loadLang(RHEA_DEFAULT_FALLOFF_LANGUAGE);
		});
}

/******************************************************************
 * lang_getCurLangISOCode
 *
 *	Ritorna il "twoCharISOLangCode" corrente
 */
Rhea.prototype.lang_getCurLangISOCode = function ()
{
	return this.Session_getValue ("lang");
}

/******************************************************************
 * lang_loadContent
 *
 *	carica lo script [nomeFileJS].js dalla cartella associata al linguaggio attualmente in uso.
 *	Ritorna una promise
 */
Rhea.prototype.lang_loadJS = function (nomeFileJS)
{
	var langISO = this.Session_getValue ("lang");
	var url = "config/lang/" +langISO +"/" +nomeFileJS +".js";
	return rheaLoadScript(url);
}
/*********************************************************
 *  In ogni momento, la GPU potrebbe segnalare degli specifici eventi degni di nota per la GUI.
 *	La pagina web può gestire questi eventi facendo "l'override" delle seguenti funzioni.
 *	Non è necessario che la pagina web gestista tutti questi eventi, può limitarsi a gestire quelli che le interessano.
 *
 *	Es:
	 	rhea.onEvent_selectionAvailabilityUpdated = function ()
			{
				alert ("sel avail updated");
			}
			
		rhea.onEvent_selectionPricesUpdated = function ()
			{
				alert ("sel prices update");
			}
 */

Rhea.prototype.onEvent_selectionAvailabilityUpdated = function()	{}

Rhea.prototype.onEvent_selectionPricesUpdated = function()	{}

Rhea.prototype.onEvent_creditUpdated = function()	{}

Rhea.prototype.onEvent_cpuMessage = function(msg, importanceLevel)	{}

Rhea.prototype.onEvent_selectionReqStatus = function(status)	{}