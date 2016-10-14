/*! JaSper v3.5 | (c) 2015 José M. Carnero (sargazos.net) | JaSper_move v2.3 */
JaSper.extend(JaSper.prototype,{move:function(a){"use strict";a=a||{},a.container=a.container||!1,a.reset=void 0==a.reset||a.reset,a.shadow=a.shadow||!1,a.restrict=a.restrict||!1,a.onMove=a.onMove||!1,a.onMoveEnd=a.onMoveEnd||!1,a.onMoveStart=a.onMoveStart||!1;var b,c={inicio:JaSper.tactil?"touchstart":"mousedown",mueve:JaSper.tactil?"touchmove":"mousemove",fin:JaSper.tactil?"touchend":"mouseup"},d=function(a){return b=JaSper.nodo.crear(a.tagName,{innerHTML:"&nbsp",class:a.className+" JaSper_shadow"}),b.style.position=a.posStyle,b.style.top=a.offsetTop,b.style.left=a.offsetLeft,b.style.height=a.clientHeight,b.style.width=a.clientWidth,b.style.border="1px dashed black",b.style.backgroundColor="#CACACA",a.parentNode.insertBefore(b,a.nextSibling),b},e=function(d,e,f){if(JaSper.event.preventDefault(d),JaSper.event.stop(d),a.reset){var g=JaSper.nodo.extend(e);e.style.left=g.posMoveStart.x-g.posMoveStart.mx+"px",e.style.top=g.posMoveStart.y-g.posMoveStart.my+"px",e.style.position=e.posStyle}if(a.shadow&&(a.reset||(e.style.position=e.posStyle,e=e.parentNode.removeChild(e),b.parentNode.insertBefore(e,b)),b.parentNode.removeChild(b)),e.style.zIndex-=10,JaSper.event.remove(document,c.mueve,f[0]),JaSper.event.remove(document,c.fin,f[1]),"function"==typeof a.onMoveEnd){var h=e.style.zIndex;e.style.zIndex=-999;var i=JaSper.move.elementFromPoint(d);e.style.zIndex=h,a.onMoveEnd.call(e,d,i)}},f=function(c,d){JaSper.event.preventDefault(c),JaSper.event.stop(c);var e=JaSper.move.posPuntero(c),f=null,g=null;if(a.shadow){b=b.parentNode.removeChild(b),g=g||d.style.zIndex,d.style.zIndex=-999,f=f||JaSper.move.elementFromPoint(c),d.style.zIndex=g;var h=JaSper.move.posObject(f),i=Math.round(h.h/2);e.y<h.y+i?f.parentNode.insertBefore(b,f):e.y>h.y2-i&&f.nextSibling?f.parentNode.insertBefore(b,f.nextSibling):f.parentNode.appendChild(b)}"function"==typeof a.onMove&&(g=g||d.style.zIndex,d.style.zIndex=-999,f=f||JaSper.move.elementFromPoint(c),d.style.zIndex=g,a.onMove.call(d,c,f));var j=JaSper.nodo.extend(d),k=j.posMoveStart.y+("x"==a.restrict?0:e.y-j.posPunteroInicial.y-j.posMoveStart.my),l=j.posMoveStart.x+("y"==a.restrict?0:e.x-j.posPunteroInicial.x-j.posMoveStart.mx);if(a.container){var m=j.posMoveStart.y2,n=j.posMoveStart.x2;k<j.posMoveStartParent.y&&(k=j.posMoveStartParent.y),l<j.posMoveStartParent.x&&(l=j.posMoveStartParent.x),m>j.posMoveStartParent.y2&&(k=j.posMoveStartParent.y2-j.posMoveStart.h),n>j.posMoveStartParent.x2&&(l=j.posMoveStartParent.x2-j.posMoveStart.w)}d.style.top=k+"px",d.style.left=l+"px",$("origen").html=JaSper.event.source(c),$("evento").html=JaSper.event.name(c)},g=function(b,g){JaSper.event.preventDefault(b),JaSper.event.stop(b);var h=1;if(b.which?h=b.which:b.button&&(h=b.button),1!=h)return!1;if("function"==typeof a.onMoveStart){var i=g.style.zIndex;g.style.zIndex=-999;var j=JaSper.move.elementFromPoint(b);g.style.zIndex=i,a.onMoveStart.call(g,b,j)}a.shadow&&d(g),JaSper.nodo.extend(g,{posMoveStart:JaSper.move.posObject(g)});var k=JaSper.nodo.extend(g,{posPunteroInicial:JaSper.move.posPuntero(b)});a.container&&(k=JaSper.nodo.extend(g,{posMoveStartParent:JaSper.move.posObject(a.container===!0?g.parentNode:a.container)}),g.parentNode.style.top=k.posMoveStartParent.y+"px",g.parentNode.style.left=k.posMoveStartParent.x+"px",g.parentNode.style.width=k.posMoveStartParent.w+"px",g.parentNode.style.height=k.posMoveStartParent.h+"px"),g.posStyle=g.style.position,g.style.position="absolute",g.style.top=k.posMoveStart.y-k.posMoveStart.my+"px",g.style.left=k.posMoveStart.x-k.posMoveStart.mx+"px",g.style.width=k.posMoveStart.w+"px",g.style.height=k.posMoveStart.h+"px",g.style.zIndex+=10;var l=function(a){f(a,g)},m=function a(b){e(b,g,[l,a])};JaSper.event.add(document,c.mueve,l),JaSper.event.add(document,c.fin,m)};this.eventAdd(c.inicio,function(a){g(a,this)})}}),JaSper.move={elementFromPoint:function(a){"use strict";if(document.elementFromPoint){var b=a.clientX,c=a.clientY,d=!0,e=0;return(e=JaSper.css.getStyle(document,"scrollTop"))>0?d=null==document.elementFromPoint(0,e+JaSper.css.getStyle(window,"height")):(e=JaSper.css.getStyle(document,"scrollLeft"))>0&&(d=null==document.elementFromPoint(e+JaSper.css.getStyle(window,"width"),0)),d||(b+=JaSper.css.getStyle(document,"scrollLeft"),c+=JaSper.css.getStyle(document,"scrollTop")),document.elementFromPoint(b,c)}return function(a){var b=a.explicitOriginalTarget;return b?(b.nodeType==Node.TEXT_NODE&&(b=b.parentNode),"HTML"==b.nodeName.toUpperCase()&&(b=document.getElementsByTagName("BODY").item(0)),b):null}(a)},posObject:function(a){"use strict";for(var b=a,c=a.offsetLeft,d=a.offsetTop;b=b.offsetParent;)c+=b.offsetLeft,d+=b.offsetTop;var e=new Array;return e.w=parseInt(JaSper.css.getStyle(a,"width")),e.h=parseInt(JaSper.css.getStyle(a,"height")),e.x=c,e.y=d,e.x2=e.x+e.w,e.y2=e.y+e.h,e.mx=parseInt(JaSper.css.getStyle(a,"marginLeft")),e.my=parseInt(JaSper.css.getStyle(a,"marginTop")),e},posPuntero:function(a){"use strict";var b=new Array;return JaSper.tactil?(b.x=a.changedTouches[0].pageX,b.y=a.changedTouches[0].pageY):navigator.userAgent.toLowerCase().indexOf("msie")>=0?(b.x=window.event.clientX+document.body.clientLeft+document.body.scrollLeft,b.y=window.event.clientY+document.body.clientTop+document.body.scrollTop):(b.x=a.clientX+window.pageXOffset,b.y=a.clientY+window.pageYOffset),b}};
