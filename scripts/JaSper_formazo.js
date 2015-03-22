/*Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
http://www.gnu.org/copyleft/gpl.html*/

/**
 * Funciones de validacion de formularios para JaSper
 * Al final script que las lanza
 */

/*traducciones*/
JaSper.funcs.extendTrads({
"en":{
	'formazo/valClave':'Check that you entered the same password in both boxes',
	'formazo/valEmail':'Invalid e-mail',
	'formazo/valFechas':'Incorrect date.\nRecommended format for your dates: ',
	'formazo/valFichero':'File type not allowed.',
	'formazo/valNif1':'NIE wrong.',
	'formazo/valNif2':'NIF wrong.',
	'formazo/valNumeros1':'Range between ',
	'formazo/valNumeros2':' and ',
	'formazo/valObligatorio':' can not be empty.',
	'formazo/valObligatorioRadio':' should have checked some option.',
	'formazo/valUrl':'Invalid URL.\nRecommended format: "http://dominio.tld"'},
"es":{
	'formazo/valClave':'Compruebe que ha escrito la misma clave en ambas casillas',
	'formazo/valEmail':'e-mail no v\u00E1lido',
	'formazo/valFechas':'Fecha incorrecta.\nFormato recomendado para las fechas: ',
	'formazo/valFichero':'Tipo de fichero no permitido.',
	'formazo/valNif1':'NIE incorrecto',
	'formazo/valNif2':'NIF incorrecto',
	'formazo/valNumeros1':'Rango entre ',
	'formazo/valNumeros2':' y ',
	'formazo/valObligatorio':' no puede estar vac\u00EDo.',
	'formazo/valObligatorioRadio':' debe tener marcada alguna opci\u00F3n.',
	'formazo/valUrl':'URL no v\u00E1lida.\nFormato recomendado: "http://dominio.tld"'}
});

JaSper.funcs.extend(JaSper.prototype, {

	/**
	 * Evita que se pierdan las opciones seleccionadas y permite que se seleccionen mas en un "select multiple" sin pulsar simultaneamente la tecla "control".
	 * 
	 * @return boolean
	 */
	/*multipleNoCtrl: function(){
		this.noPropagarEvento();
		this.each(function (){
				//this.options[this.selectedIndex].selected = !this.options[this.selectedIndex].selected;
				//return false;

				var seleccionadas = new Array();
				for(var i=0; i<this.options.length; i++){if(this.options[i].selected == true) seleccionadas.push(i);}
				var actual = this.selectedIndex;

				for (var i=0; i<seleccionadas.length; i++)
				{
					if (actual == seleccionadas[i])
					{
						seleccionadas.splice(i, 1);
						break;
					}
				}

				if (i >= seleccionadas.length) seleccionadas.push(actual);
				for (var i=0; i<this.options.length; i++) this.options[i].selected = false;
				for (var i=0; i<seleccionadas.length; i++) this.options[seleccionadas[i]].selected = true;
			}
		);
	},*/

	 /**
	 * \~spanish Llena un select existente con los valores pasados.
	 * Devuelve falso si errores.
	 * \~english Fills select with "valores", "etiquetas".
	 * false on errors.\~
	 * 
	 * @param array valores \~spanish Valores de los option\~english Options values\~
	 * @param array etiquetas \~spanish Etiquetas de los option, en el mismo orden que el anterior\~english Labels for options, order corresponence with previous\~
	 * @param string valor \~spanish Valor seleccionado\~english Selected value\~
	 * @return boolean
	 */
	llena_select: function (valores, etiquetas, valor){
		if(!valores) return(false); //TODO crear el select si no existe.

		if(!etiquetas){ //no se ha pasado array de etiquetas
			var etiquetas = new Array();
			etiquetas = valores;
		}
		if(!valor) var valor = null;

		this.each(function (vls, eti, val){
			//if(this.type != 'select-one') return(false); //solo con select no multiple
			this.innerHTML = ''; //elemento.length = 0; //limpiando el select; length = 0 no limpia optgroup
			for(i=0;i < vls.length;i++){
				var optNew = document.createElement('option');
				optNew.text = eti[i];
				optNew.value = vls[i];
				if(vls[i] == val) optNew.selected = true;
				try{
					this.add(optNew, this.options[this.length]); // standards compliant; doesn't work in IE
				}
				catch(ex){
					this.add(optNew, this.length); // IE only
				}
			}
		}, new Array(valores, etiquetas, valor));

		return(this);
	},

	//limpiar los elementos del formulario
	limpiaForm: function (){
		this.each(function (){
			var oForm = (this.tagName.toLowerCase() == 'form') ? this.elements : (this.form) ? this.form.elements : false;
			if(oForm === false) return(false); //no es un formulario ni elemento de formulario

			for(i=0;i<oForm.length;i++){
				switch(oForm[i].type){
					case 'text':
					case 'password':
					case 'textarea':
						oForm[i].value = '';
						break;
					case 'select-one':
						oForm[i].selectedIndex = 0;
						break;
					case 'checkbox':
						oForm[i].checked = false;
						break;
					default:
						//alert(oForm[i].type);
						return(false);
				}
			}
		});
		return(this);
	}

});

//validaciones
JaSper.funcs.extend(JaSper.prototype, {

	/**
	 * \~spanish Validacion de campos password
	 * no permite encadenado de metodos
	 * \~english Password validation\~
	 * 
	 * @return string
	 */
	valClave: function (objId2){
		if(!objId2) return false; //no se comprueba el campo de clave desde el de confirmacion
		var bRet = true;

		if(typeof objId2 == "string") objId2 = document.getElementById(objId2);

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != '' && objId2.value != '' && this.nodes[i].value != objId2.value) bRet = false;
		}

		if(bRet) return false; //pass valida
		else return(JaSper.funcs._t('formazo/valClave')); //pass no valida
	},

	/**
	 * \~spanish Validacion de campos e-mail
	 * Devuelve booleano; no permite encadenado de metodos
	 * \~english E-mail validation\~
	 * 
	 * @return boolean
	 */
	valEmail: function (){
		var bRet = true, filtro = /^([a-zA-Z0-9_\.\-])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/;

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != ''){
				if(filtro.test(this.nodes[i].value)) bRet = bRet & true; //e-mail valido
				else bRet = bRet & false; //e-mail no valido
			}
			else bRet = bRet & true;
		}

		if(bRet) return false;
		else return(JaSper.funcs._t('formazo/valEmail')); //e-mail no valido
	},

	/**
	 * \~spanish Validacion de campos de fechas
	 * Comprueba que una fecha introducida es valida y la convierte al formato aaaa-mm-dd hh:mm:ss; no distingue formato europeo de americano, puede dar errores en esa confusion
	 * o permite encadenado de metodos
	 * \~english Date validation
	 * Convert date to format aaaa-mm-dd hh:mm:ss; could make mistakes with european format and american format\~
	 * 
	 * @param string formato \~spanish Formato de la fecha\~english Date format\~
	 * @return boolean
	 */
	valFechas: function (formato){
		if(!formato) formato = 'aaaa-mm-dd hh:mm:ss';
		var bRet = true;

		for(var i=0;i<this.nodes.length;i++){
			var bRetInt = false; //fecha incorrecta
			if(this.nodes[i].value != ''){
				var dFecha = this.nodes[i].value.replace(/-/g, '/');
				var fechaInt = Date.parse(dFecha);
				if(!isNaN(fechaInt)){
					var fecha = new Date();
					fecha.setTime(fechaInt);
					if(fecha != 'Invalid Date'){
						bRetInt = true; //fecha correcta
					}
				}
			}

			bRet = bRet & bRetInt;
		}

		if(bRet) return false;
		else return(JaSper.funcs._t('formazo/valFechas') + formato); //fecha incorrecta
	},

	/**
	 * \~spanish Validacion de campos fichero, comprueba la extension del archivo entre las permitidas
	 * no permite encadenado de metodos
	 * \~english File fields validation; verify file extension\~
	 * 
	 * @param string extensiones \~spanish Extensiones permitidas\~english Allowed extensions\~
	 * @return boolean
	 */
	valFichero: function (extensiones){
		var bRet = true;

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != ''){
				var aTemp = new Array();
				aTemp = this.nodes[i].value.split('.');
	
				if(extensiones.indexOf(aTemp[aTemp.length - 1].toLowerCase()) >= 0) bRet = bRet & true; //tipo de fichero correcto
				else bRet = bRet & false; //tipo de fichero no permitido
			}
			else bRet = bRet & true;
		}

		if(bRet) return false;
		else return(JaSper.funcs._t('formazo/valFichero')); //tipo de fichero no permitido
	},

	/**
	 * \~spanish Validacion de campos NIF/NIE
	 * NIF sin espacios, 10 caracteres (NIE) maximo
	 * no permite encadenado de metodos
	 * \~english NIF/NIE validation
	 * NIF without spaces, 10 characters (NIE) maximun\~
	 * 
	 * @return boolean
	 */
	valNif: function (){
		var bRet = true, mensaje = '';

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != ''){
				this.nodes[i].value = this.nodes[i].value.toUpperCase();

				dni = this.nodes[i].value.substr(0, this.nodes[i].value.length-1); //NIF
				if(this.nodes[i].value.charAt(0) == 'X') dni = dni.substr(1); //NIE

				letra = this.nodes[i].value.charAt(this.nodes[i].value.length-1);

				if(dni.length == 8 && isNaN(letra)){
					var control = 'TRWAGMYFPDXBNJZSQVHLCKE';
					pos = dni % 23;
					control = control.charAt(pos);

					if(control==letra) bRet = bRet & true; //nif correcto
				}
				else{
					if(this.nodes[i].value.charAt(0) == 'X') mensaje += JaSper.funcs._t('formazo/valNif1'); //NIE
					else mensaje += JaSper.funcs._t('formazo/valNif2'); //NIF
					bRet = bRet & false; //nif incorrecto
				}
			}
			else bRet = bRet & true;
		}

		if(bRet) return false;
		else return mensaje;
	},

	/**
	 * \~spanish Validacion de numericos, con rango entre "menor" y "mayor" (opcionales ambos)
	 * no permite encadenado de metodos
	 * \~english Number validation; minimun and maximun range optional\~
	 * 
	 * @param integer menor \~spanish Rango minimo\~english Minimun range\~
	 * @param integer mayor \~spanish Rango máximo\~english Maximun range\~
	 * @return boolean
	 */
	valNumeros: function (menor, mayor){
		var bRet = true;

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != ''){
				if(typeof menor == 'undefined')
					menor = 0;
				if(typeof mayor == 'undefined')
					mayor = Math.pow(10, this.nodes[i].maxLength);

				num = parseInt(this.nodes[i].value);

				if((num >= menor && num <= mayor) || this.value == '') bRet = bRet & true;
				else bRet = bRet & false;
			}
			else bRet = bRet & true;
		}

		if(bRet) return false;
		else return(JaSper.funcs._t('formazo/valNumeros1') + menor + JaSper.funcs._t('formazo/valNumeros2') + mayor); //numero incorrecto
	},

	/**
	 * \~spanish Campo de caracter obligatorio, no puede tener valor nulo
	 * Se llama en el envio del formulario
	 * no permite encadenado de metodos
	 * \~english Required field, can't be null\~
	 * 
	 * @return boolean
	 */
	valObligatorio: function (){
		//cssValorObjeto('comunes.css', this);
		var bRet = true, text = [];

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != '') bRet = bRet & true;
			else{
				text[i] = $('label', this.nodes[i].parentNode).text(); //title for error container
				bRet = bRet & false;
			}
		}

		if(bRet) return false;
		else return('"' + text.join('", "') + '"' + JaSper.funcs._t('formazo/valObligatorio'));
	},

	/*version para radio buttons*/
	valObligatorioRadio: function (text){
		var bRet = true;

		for(var j=0;j<this.nodes.length;j++){
			var cnt = -1, objName = this.nodes[j];
			for(var i=objName.length-1; i > -1; i--){
				if(objName[i].checked){cnt = i; i = -1;}
			}

			if(cnt > -1) bRet = bRet & true;
			else{
				text[i] = $('label', this.nodes[i].parentNode).text(); //title for error container
				bRet = bRet & false;
			}
		}

		if(bRet) return false;
		else return('"' + text.join('", "') + '"' + JaSper.funcs._t('formazo/valObligatorioRadio'));
	},

	/**
	 * \~spanish Fuerza la entrada de solo numeros barra (/) y guion (-)
	 * Llamar con: onkeypress="return $(this).valTeclasFechas(event);"
	 * o '$('#objId').eventAdd("keypress", function (e){this.valTeclasFechas(e);});'
	 * no permite encadenado de metodos
	 * \~english Filter keyboard enter, allow only numbers, "/" and "-"
	 * $('#objId').eventAdd("keypress", function (e){this.valTeclasFechas(e);});\~
	 * 
	 * @param event e Event
	 * @return boolean
	 */
	valTeclasFechas: function (e){
		var bRet = true;
		var e = e || window.event;

		var char_code = JaSper.funcs.key_code(e);
		for(var i=0;i<this.nodes.length;i++){
			//permite la entrada de numeros, espacio, dos puntos, barra y guion
			if(charCode > 31 && (charCode < 48 || charCode > 58) && charCode != 32 && charCode != 47 && charCode != 45){
				bRet = bRet & false; //alert("No son caracteres de fecha");
				_JaSper(this.nodes[i]).noPropagarEvento(e); //evita la pulsacion
			}
			else{
				bRet = bRet & true;
				if(this.nodes[i].value.indexOf(' ') != -1 && charCode == 32) bRet = bRet & false;
			}
		}

		return(!bRet);
	},

	/**
	 * \~spanish Fuerza la entrada de solo numeros y coma decimal
	 * Llamar con: onkeypress="return valTeclasNumeros(event);"
	 * o '$('#objId').eventAdd("keypress", function (e){this.valTeclasNumeros(e);});'
	 * no permite encadenado de metodos
	 * \~english Filter keyboard enter, allow only numbers and "."
	 * $('#objId').eventAdd("keypress", function (e){this.valTeclasNumeros(e);});\~
	 * 
	 * @param event e \~spanish Evento\~english Event\~
	 * @param boolean decimal true\~spanish permite punto decimal\~english allow decimal point\~
	 * @return boolean
	 */
	valTeclasNumeros: function (e, decimal){
		var bRet = true;
		var e = e || window.event;
		var decimal = typeof(decimal) != 'undefined' ? decimal : true;

		var char_code = JaSper.funcs.key_code(e);
		for(var i=0;i<this.nodes.length;i++){
			if(decimal){ //permite la entrada de numeros y punto decimal
				if(char_code > 31 && (char_code < 48 || char_code > 57) && char_code != 46) bRet = bRet & false; //alert("no es un numero")
				else{
					bRet = bRet & true;
					if(this.nodes[i].value.indexOf('.') != -1 && char_code == 46) bRet = bRet & false;
				}
			}
			else{ //permite la entrada de numeros sin punto decimal
				if(char_code > 31 && (char_code < 48 || char_code > 57)) bRet = bRet & false; //alert("no es un numero")
				else bRet = bRet & true;
			}
		}

		return(!bRet);
	},
	/*function containsNonLatinCodepoints(s) {
		return /[^\u0000-\u00ff]/.test(s);
	}*/

	/**
	 * \~spanish validacion de campos URL
	 * \~english URL validation\~
	 */
	valUrl: function (){
		var bRet = true;
		var filtro = /(((ht|f)tp(s?):\/\/)|(www\.[^ [\]()\n\r\t]+)|(([012]?[0-9]{1,2}\.){3}[012]?[0-9]{1,2})\/)([^ [\](),;"'<>\n\r\t]+)([^. [\](),;"'<>\n\r\t])|(([012]?[0-9]{1,2}\.){3}[012]?[0-9]{1,2})/;
		//var filtro = /((ht|f)tp(s?):\/\/)([^ [\](),;"'<>\n\r\t]+)([^. [\](),;"'<>\n\r\t])/; //fuerza a que haya "http://" (o lo que corresponda) al principio

		for(var i=0;i<this.nodes.length;i++){
			if(this.nodes[i].value != ''){
				if(filtro.test(this.nodes[i].value)) bRet = bRet & true; //url valida
				else bRet = bRet & false; //url no valida
			}
			else bRet = bRet & true;
		}

		if(bRet) return false;
		else return(JaSper.funcs._t('formazo/valUrl'));
	}

});

/*
 * Validacion, sobre una idea de Andrés Nieto
 * http://www.anieto2k.com/2008/06/25/validar-formularios-con-jquery/
 */

//validaciones
var filters = {
	frmOblig: function (el){return ($(el).valObligatorio(el.id));},
	clave: function (el){return ($(el).valClave(document.getElementById('objId2')));},
	email: function (el){return ($(el).valEmail());},
	entero: function (el){return ($(el).valNumeros());}, //numerico de tipo entero, no permite decimales ni separadores de miles, solo numeros
	fichero: function (el){return ($(el).valFichero(el.id));},
	fecha: function (el){return ($(el).valFechas());},
	nif: function (el){return ($(el).valNif());},
	numerico: function (el){return ($(el).valNumeros());}, //permite decimales, con "."
	telefono: function (el){return ($(el).valNumeros());},
	url: function (el){return ($(el).valUrl());}
};
var filters_keys = {
	entero: function (e, el){return ($(el).valTeclasNumeros(e, false));}, //numerico de tipo entero, no permite decimales ni separadores de miles, solo numeros
	fecha: function (e, el){return ($(el).valTeclasFechas(e));},
	numerico: function(e, el){return ($(el).valTeclasNumeros(e, true));} //permite decimales, con "."
};

$(document).ready(function(){
	$("<form>").each(function(ev){
		var var_depende = false;
		var var_ligado = false;
		var el = this;

		try{var_depende = eval(el.id.replace('frm', 'depende_'));}
		catch(e){/*return*/;}
		try{var_ligado = eval(el.id.replace('frm', 'ligado_'));}
		catch(e){/*return*/;}

		//dependencias
		if(var_depende !== false){
			for(dep in var_depende){
				$(dep).eventAdd("change", function (){
					for(var i = 0;i < var_depende[dep].length;i++){
						if(this.value == var_depende[dep][i][1]) $('#' + var_depende[dep][i][0]).setStyle('display', '');
						else $('#' + var_depende[dep][i][0]).setStyle('display', 'none');
					}
				});
			}
		}
		//ligaduras
		if(var_ligado !== false){
			for(lig in var_ligado){
				var val_ligado = null;
				var eti_ligado = null;

				try{
					val_ligado = eval('valores_' + lig);
					eti_ligado = eval('etiquetas_' + lig);
				}
				catch(e){}

				$('#' + lig).llena_select(val_ligado[document.getElementById(var_ligado[lig]).selectedIndex], eti_ligado[document.getElementById(var_ligado[lig]).selectedIndex], document.getElementById(var_ligado[lig]).value);
				$('#' + var_ligado[lig]).eventAdd('change', function(e){
					$('#' + lig).llena_select(val_ligado[document.getElementById(var_ligado[lig]).selectedIndex], eti_ligado[document.getElementById(var_ligado[lig]).selectedIndex], document.getElementById(var_ligado[lig]).value);
				});
			}
		}

		//bloqueos de teclas
		if(typeof filters_keys != 'undefined'){
			$('<input>,<textarea>', this).each(function(e){
				var el = this;
				if(el.className != 'undefined'){
					var csplit = el.className.split(" ");
					for(klass in csplit){
						if(JaSper.funcs.isFunction(filters_keys[csplit[klass]])){
							var fun = csplit[klass];
							$(el).eventAdd('keydown', function (e){
								if(filters_keys[fun](e, el))
									{JaSper.funcs.eventStop(e || window.event);return(false);};
								//alert(filters_keys[fun](e, el) + ' - ' + e.keyCode + ' - ' + this.id);
							});
						}
					}
				}
			});
		}

		//secciones repetidas
		/*$('.frmRepeat', this).each(function(e, el){
			//boton de clonar
			jQuery('<span/>', {
				id: el.id + 'Clone',
				title: 'Clone this',
				text: 'Clone',
				class: 'tools',
				css: {cursor:'pointer'}
			}).appendTo(el.firstChild).bind('click', function (){
				//alert(this.parentNode.parentNode.id);
				//$(this.parentNode.parentNode).clone().insertAfter(this.parentNode.parentNode).removeAttr("id").find("*").removeAttr("id").remove(this.parentNode.firstChild);
				$('#repeat_' + el.id).clone(false).append(
					//boton de borrado
					jQuery('<span/>', {
						id: el.id + 'Remove',
						title: 'Remove this',
						text: 'Remove',
						class: 'tools',
						css: {cursor:'pointer'}
					}).bind('click', function (){$(this.parentNode).remove();})
				).appendTo(this.parentNode.parentNode).find('*').removeAttr("id").find('input').val('').end();
			});
		});*/

	});

	//validacion al envio
	$('<form>').eventAdd('submit', function(e){
		window.errMens = [];

		if(typeof filters == 'undefined') return;
		$('<input>,<textarea>,<select>', this).each(function(x){
			var el = this;
			if(el.className != 'undefined'){
				var csplit = el.className.split(" ");
				for(klass in csplit) if(JaSper.funcs.isFunction(filters[csplit[klass]])) var errTemp = filters[csplit[klass]](el);
				if(errTemp){
					$(el).addClass("frmError");
					window.errMens[window.errMens.length] = errTemp;
				}
				else $(el).removeClass("frmError");
			}
		});

		if($('.frmError', this).length > 0){
			JaSper.funcs.eventStop(e || window.event);

			/*$('.frmError').each( //marcar los errores
						function(){alert(this.name + ' error');}
					);*/
			alert("Se han producido los siguientes errores:\n\n" + window.errMens.join("\n"));
			return false;
		}

		return true;
	});
});

/*$aJavascript['validaciones'] = array('-' => array('', ''),
		'clave' => array('$(\'#objId\').eventAdd("change", function (){$(this).valClave(document.getElementById(\'objId2\'));});'."\n".'$(\'#objId2\').addEvent("change", function (){$(this).valClave(document.getElementById(\'objId\'));});'."\n", '$(\'#objId\').valClave(document.getElementById(\'objId2\'))'),
		'email' => array('$(\'#objId\').eventAdd("change", function (){$(this).valEmail();});'."\n", '$(\'#%s\').valEmail() '),
		'entero' => array('$(\'#objId\').eventAdd("keypress", function (e){if(!$(this).valTeclasNumeros(e, false)) if(e.preventDefault){e.preventDefault();} else{event.returnValue = false;} });'."\n".'$(\'#objId\').addEvent("change", function (){$(this).valNumeros();});'."\n", '$(\'#%s\').valNumeros() '), //numerico de tipo entero, no permite decimales ni separadores de miles, solo numeros
		'fichero' => array('$(\'#objId\').eventAdd("change", function (){$(this).valFichero(\'valores\');});'."\n", '$(\'#%s\').valFichero(\'%s\') '),
		'fecha' => array('$(\'#objId\').eventAdd("keypress", function (e){if(!$(this).valTeclasFechas(e)) if(e.preventDefault){e.preventDefault();} else{event.returnValue = false;} });'."\n".'$(\'#objId\').addEvent("change", function (){$(this).valFechas();});'."\n", '$(\'#%s\').valFechas() '),
		'nif' => array('$(\'#objId\').eventAdd("change", function (){$(this).valNif();});'."\n", '$(\'#%s\').valNif() '),
		'numerico' => array('$(\'#objId\').eventAdd("keypress", function (e){if(!$(this).valTeclasNumeros(e, true)) if(e.preventDefault){e.preventDefault();} else{event.returnValue = false;} });'."\n".'$(\'#objId\').addEvent("change", function (){$(this).valNumeros();});'."\n", '$(\'#%s\').valNumeros() '), //permite decimales, con "."
		'telefono' => array('$(\'#objId\').eventAdd("keypress", function (e){if(!$(this).valTeclasNumeros(e, false)) if(e.preventDefault){e.preventDefault();} else{event.returnValue = false;} });'."\n".'$(\'#objId\').addEvent("change", function (){$(this).valNumeros();});'."\n", '$(\'#%s\').valNumeros() '),
		'url' => array('$(\'#objId\').eventAdd("change", function (){$(this).valUrl();});'."\n", '$(\'#%s\').valUrl() '),
		'obligatorio' => array("\n", '$(\'#%s\').valObligatorio(\'%s\') ') //validacion de campos obligatorios
	);

	$aJavascript['frmBtnSubmitDel'] = '$(\'#frmButtonUpdDel%s\').addEvent(\'click\', function (){
		if(confirm(\'%s\')){
			document.getElementById(\'frmAccion%s\').value = \'del\';
			window.bSinsalvar = false;
			document.getElementById(\'frm%s\').submit();
		}
		else return(false);
	});'."\n";
	$aJavascript['frmBtnSubmitDel2'] = '$(\'#frmBtnSubmit%s\').eventAdd(\'click\', function (){document.getElementById(\'frmAccion%s\').value = \'upd\';});'."\n";
	$aJavascript['frmBtnLimpia'] = '$(\'#frmBtnLimpia%s\').eventAdd(\'click\', function(){$(this).limpiaForm();});'."\n";
	$aJavascript['frmBtnCancelar'] = '$(\'#frmButtonCancelar%s\').eventAdd(\'click\', function (){history.back(1);});'."\n";
	$aJavascript['validacionIL'] = '$(\'#frm%s\').eventAdd(\'submit\', function (e){if(!val_%s()){if(e.preventDefault){e.preventDefault();}else{event.returnValue = false;}}else{window.bSinsalvar = false;}});'."\n";
	$aJavascript['sTempJs1'] = 'window.bSinsalvar = false;'."\n".'$(this).eventAdd(\'beforeunload\', function (e){if(!window.bSinsalvar) return true;var e = e || window.event;if(e){e.returnValue = "%s";}return "%s";});'."\n";
	$aJavascript['sTempJs2'] = '$(\'#%s\').eventAdd(\'click\', function (){window.bSinsalvar = true;});'."\n";
	$aJavascript['frmValidar'] = '<script type="text/javaScript"><!--
	%s
	$(document).ready(function (){
		$("#frm'.$a.'").validate(
			rules: {'.$a.'}
		);
	%s
	});
	--></script>'."\n";
	$aJavascript['fDepende_radio'] = '$("@%s").eventAdd("change", function (){if(this.checked && this.value == "%s") $(\'#%s\').setStyle(\'display\', \'\');else $(\'#%s\').setStyle(\'display\', \'none\');});';
	$aJavascript['fDepende'] = '$("#%s").eventAdd("change", function (){if(this.value == "%s") $(\'#%s\').setStyle(\'display\', \'\');else $(\'#%s\').setStyle(\'display\', \'none\');});';
	//$oForm->setJavascript($aJavascript);
*/
