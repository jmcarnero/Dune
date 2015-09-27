/*! JaSper v3.4 | (c) 2015 José M. Carnero (sargazos.net) | JaSper_datetime v1.0 */
"use strict";JaSper.extend(JaSper.prototype,{countdown:function(e,t){e=e||Date.now()/1e3+100;var a=function(e,a,n){return t?(e=e||null,t.apply(e,[a,n])):void 0},n=function(e,t){var r={seg:0,min:0,hor:0,dia:0,sem:0},o={seg:0,min:0,hor:0,dia:0,sem:0};r.seg=t-Date.now()/1e3,o.seg=Math.floor(r.seg%60),r.min=r.seg/60,o.min=Math.floor(r.min%60),r.hor=r.min/60,o.hor=Math.floor(r.hor%24),r.dia=r.hor/24,o.dia=Math.floor(r.dia%30),r.seg&&(e.seg.innerHTML=o.seg,a(e.seg,o.seg,60)),r.min&&(e.min.innerHTML=o.min,a(e.min,o.min,60)),r.hor&&(e.hor.innerHTML=o.hor,a(e.hor,o.hor,24)),r.dia&&(e.dia.innerHTML=Math.floor(r.dia),a(e.dia,Math.floor(r.dia),365));setTimeout(function(){n(e,t)},1e3)};return this.each(function(){var t=JaSper.nodo.crear("p",{"class":"JaSper countdown"},this),a={dia:JaSper.nodo.crear("span",{"class":"dias"},t),hor:JaSper.nodo.crear("span",{"class":"horas"},t),min:JaSper.nodo.crear("span",{"class":"minutos"},t),seg:JaSper.nodo.crear("span",{"class":"segundos"},t)};n(a,e)}),this},datePicker:function(e){e=e||{},e.contenedor=e.contenedor||"JaSperDatePicker",e.fechaMin=new Date(e.fechaMin||"1970-01-01"),e.fechaIni=new Date(e.fechaIni||new Date),e.fechaMax=new Date(e.fechaMax||"2222-01-01"),e.formato=e.formato||"yyyy-mm-dd",e.semana=e.semana||["Lunes","Martes","Miercoles","Jueves","Viernes","Sábado","Domingo"],e.semanaCorta=e.semanaCorta||["L","M","X","J","V","S","D"],e.mesesCorto=e.mesesCorto||["Ene","Feb","Mar","Abr","May","Jun","Jul","Ago","Sep","Oct","Nov","Dic"];var t=function(e,t,a){var n=JaSper.nodo.crear("div",{innerHTML:t,"class":a});return 1==e.nodeType?e.parentNode.appendChild(n):void 0},a=function(t){var a={t:t,a:t.getFullYear(),m:t.getMonth()+1,d:t.getDate()},n=a.a+"-"+("00"+(a.m-1)).slice(-2)+"-01";if(1==a.m)var n=a.a-1+"-12-01";var r=a.a+"-"+("00"+(a.m+1)).slice(-2)+"-01";if(12==a.m)var r=a.a+1+"-01-01";return[n,a.a+" - "+e.mesesCorto[a.m],r]},n=function(t){var a={t:t,a:t.getFullYear(),m:t.getMonth()+1,d:t.getDate()},n=new Date(a.a,a.m-1,1).getDay()-1;n=0>n?6:n;var r=JaSper.datetime.diasEnMes(a.t),o=[a.a,a.t.getMonth()-1];0>o&&(o[0]=o[0]--,o[1]=11);for(var i=JaSper.datetime.diasEnMes(o[0],o[1]),s=" data-"+e.contenedor,d="",c=-n;r>c;){for(var p="<tr>",u=0;u<e.semanaCorta.length;u++){var m,l=a.a,h=a.m,f=c+u+1;0>=f?(f=i+f,--h<1&&(l--,h=12),m="mesPasado"):f>r?(f-=r,++h>12&&(l++,h=1),m="mesSiguiente"):m="mesActual",p+='<td class="'+m+'"'+s+'="'+(l+"-"+h+"-"+f)+'">'+f+"</td>"}c+=u,p+="</tr>",d+=p}return d},r=function(t){var r=({t:t,a:t.getFullYear(),m:t.getMonth()+1,d:t.getDate()}," data-"+e.contenedor),o=a(t),i="<table>";i+='<caption><span class="butMesAnterior"'+r+'="'+o[0]+'">&lt;&lt;</span>&nbsp;&nbsp;&nbsp;<span class="mesActual">'+o[1]+'</span>&nbsp;&nbsp;&nbsp;<span class="butMesSiguiente"'+r+'="'+o[2]+'">&gt;&gt;</span></caption>',i+="<thead><tr>";for(var s=0;s<e.semanaCorta.length;s++)i+="<td>"+e.semanaCorta[s]+"</td>";return i+="</tr></thead>",i+="<tbody>",i+=n(t),i+="</tbody>",i+="</table>"};return this.each(function(e,r){var o="data-"+r,i=this,s=t(this,e,r);JaSper(i).eventAdd("click",function(){s.style.display="block"});var d=function(){s.style.display="none",i.value=JaSper.nodo.attrib(this,o)};JaSper("tbody td.mesPasado, tbody td.mesActual, tbody td.mesSiguiente",s).eventAdd("click",d),JaSper("caption span.butMesAnterior, caption span.butMesSiguiente",s).eventAdd("click",function(){JaSper("tbody td.mesPasado, tbody td.mesActual, tbody td.mesSiguiente",s).eventRemove("click",d);var e=new Date(JaSper.nodo.attrib(this,o));JaSper("tbody",s).html(n(e));var t=a(e);JaSper("caption span.butMesAnterior",s).attrib(o,t[0]),JaSper("caption span.mesActual",s).html(t[1]),JaSper("caption span.butMesSiguiente",s).attrib(o,t[2]),JaSper("tbody td.mesPasado, tbody td.mesActual, tbody td.mesSiguiente",s).eventAdd("click",d)})},[r(e.fechaIni),e.contenedor]),this},timePicker:function(e){return e=e||{},this.each(function(){return JaSper.anim.fade(this,oTipo,iMiliSec)}),this}}),JaSper.datetime={},JaSper.extend(JaSper.datetime,{diasEnMes:function(e,t){if(JaSper.datetime.esFecha(e)&&(t=e.getMonth(),e=e.getFullYear()),!JaSper.funcs.isNumber(e)||!JaSper.funcs.isNumber(t))return!1;var a=Math.ceil((t-11)/12);return t=a>0?11:0>a?0:t,e+=a,[31,JaSper.datetime.esBisiesto(e)?29:28,31,30,31,30,31,31,30,31,30,31][t]},esBisiesto:function(e){return e%4===0&&e%100!==0||e%400===0},esFecha:function(e){return/Date/.test(Object.prototype.toString.call(e))&&!isNaN(e.getTime())},esFinde:function(e){var t=e.getDay();return 0===t||6===t}});
