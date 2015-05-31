/*Copyright (C) 2015 José M. Carnero <jm_carnero@sargazos.net>

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

'use strict';

/*****************************************************
* Funciones de validacion de formularios para JaSper *
*****************************************************/

/*traducciones*/
_JaSper.funcs.extendTrads({
"en":{
	'valida/bic':'BIC/SWIFT bank number invalid',
	'valida/clave':'Check that you entered the same password in both boxes',
	'valida/email':'Invalid e-mail',
	'valida/fechas':'Incorrect date.\nRecommended format for your dates: ',
	'valida/fichero':'File type not allowed.',
	'valida/iban':'IBAN bank number invalid',
	'valida/nif1':'NIE wrong.',
	'valida/nif2':'NIF wrong.',
	'valida/numeros1':'Range between ',
	'valida/numeros2':' and ',
	'valida/obligatorio':' can not be empty.',
	'valida/obligatorioRadio':' should have checked some option.',
	'valida/url':'Invalid URL.\nRecommended format: "http://dominio.tld"'},
"es":{
	'valida/bic':'Código bancario BIC/SWIFT incorrecto',
	'valida/clave':'Compruebe que ha escrito la misma clave en ambas casillas',
	'valida/email':'e-mail no v\u00E1lido',
	'valida/fechas':'Fecha incorrecta.\nFormato recomendado para las fechas: ',
	'valida/fichero':'Tipo de fichero no permitido.',
	'valida/iban':'Código bancario IBAN incorrecto',
	'valida/nif1':'NIE incorrecto',
	'valida/nif2':'NIF incorrecto',
	'valida/numeros1':'Rango entre ',
	'valida/numeros2':' y ',
	'valida/obligatorio':' no puede estar vac\u00EDo.',
	'valida/obligatorioRadio':' debe tener marcada alguna opci\u00F3n.',
	'valida/url':'URL no v\u00E1lida.\nFormato recomendado: "http://dominio.tld"'}
});

_JaSper.funcs.extend(_JaSper.prototype, {

	/**
	 * Valida formularios
	 * @param object oProps Propiedades para las validaciones
	 * return object JaSper
	 */
	validar: function(props){
		var props = props || {}; //configuracion del metodo
		props.classError = props.classError || 'frmError'; //configuracion del metodo
		

		var filters = { //validaciones
			frmObligatorio: function (el){return _JaSper.valida.obligatorio(el);},
			clave: function (el){return _JaSper.valida.clave(el, document.getElementById('objId2'));},
			frmEmail: function (el){return _JaSper.valida.email(el);},
			entero: function (el){return _JaSper.valida.numeros(el);}, //numerico de tipo entero, no permite decimales ni separadores de miles, solo numeros
			fichero: function (el){return _JaSper.valida.fichero(el);},
			fecha: function (el){return _JaSper.valida.fechas(el);},
			nif: function (el){return _JaSper.valida.nif(el);},
			frmNumerico: function (el){return _JaSper.valida.numeros(el);}, //permite decimales, con "."
			telefono: function (el){return _JaSper.valida.numeros(el);},
			url: function (el){return _JaSper.valida.url(el);}
		};
		var filters_keys = {
			entero: function (ev, el){return _JaSper.valida.teclasNumeros(el, ev, false);}, //numerico de tipo entero, no permite decimales ni separadores de miles, solo numeros
			fecha: function (ev, el){return _JaSper.valida.teclasFechas(el, ev);},
			frmNumerico: function(ev, el){return _JaSper.valida.teclasNumeros(el, ev, true);} //permite decimales, con "."
		};

		//bloqueos de teclas
		this.each(function (){ //se busca en cada formulario los elementos bloqueables
			JaSper('<input>,<textarea>', this).each(function(ev){
				var el = this;
				if(el.className != 'undefined'){
					var csplit = el.className.split(" ");
					for(var i = 0; i < csplit.length; i++){
						if(_JaSper.funcs.isFunction(filters_keys[csplit[i]])){
							var fun = csplit[i];
							JaSper(el).eventAdd('keydown', function (ev){
								if(filters_keys[fun](ev, el)){
									_JaSper.event.preventDefault(ev);
									_JaSper.event.stop(ev);
									return false;
								}
								//alert(filters_keys[fun](e, el) + ' - ' + e.keyCode + ' - ' + this.id);
							});
						}
					}
				}
			});
		});

		//validacion al envio
		this.eventAdd('submit', function(ev){
			window.errMens = [];

			if(typeof filters == 'undefined') return;
			JaSper('<input>,<textarea>,<select>', this).each(function(x){
				var el = this;
				if(el.className != 'undefined'){
					var csplit = el.className.split(" ");
					var aErrTemp = [];
					for(var i = 0; i < csplit.length; i++){
						if(_JaSper.funcs.isFunction(filters[csplit[i]])){
							var errTemp = filters[csplit[i]](el);
							if(errTemp !== false)
								aErrTemp[aErrTemp.length] = filters[csplit[i]](el);
						}
					}

					if(aErrTemp.length){
						JaSper(el).addClass(props.classError);
						window.errMens[window.errMens.length] = aErrTemp.join("\n");
					}
					else
						JaSper(el).removeClass(props.classError);
				}
			});

			if(JaSper('.' + props.classError, this).length > 0){
				_JaSper.event.preventDefault(ev);
				_JaSper.event.stop(ev);

				/*JaSper('.' + props.classError).each( //marcar los errores
							function(){alert(this.name + ' error');}
						);*/
				alert("Se han producido los siguientes errores:\n\n" + window.errMens.join("\n"));
				return false;
			}

			return true;
		});

		return this;
	}
});

_JaSper.valida = {};

//validaciones
_JaSper.funcs.extend(_JaSper.valida, {

	/**
	 * validador BIC (Business Identifier Codes) o SWIFT
	 *
	 * @param object sBic Objeto con codigo BIC a validar
	 * @return boolean
	 */
	bic: function(oBic){
		var bRet = true;

		oBic.value = oBic.value.toString().trim() || '';
		if(!oBic.value) //no se hacen mas comprobaciones, vacio no es BIC
			bRet = false;

		if(! /^[a-z]{6}[0-9a-z]{2}([0-9a-z]{3})?$/i.test(oBic.value)){
			bRet = false;
		}

		if(bRet) return false; //pass valida
		else return(_JaSper.funcs._t('valida/bic')); //pass no valida
	},

	/**
	 * Validacion de campos password
	 * 
	 * @param object oClave Campo de clave original
	 * @param object objId2 Id del campo de clave repetido
	 * @return string
	 */
	clave: function (oClave, oClave2){
		if(!oClave2)
			return false; //no se comprueba el campo de clave desde el de confirmacion

		var bRet = true;


		if(!oClave && oClave2.value != '' && oClave != oClave2.value)
			bRet = false;

		if(bRet) return false; //pass valida
		else return(_JaSper.funcs._t('valida/clave')); //pass no valida
	},

	/**
	 * Validacion de campos e-mail
	 *
	 * @param object oEmail Email a validar
	 * @return boolean
	 */
	email: function (oEmail){
		var bRet = true, filtro = /^([a-zA-Z0-9_\.\-])+\@(([a-zA-Z0-9\-])+\.)+([a-zA-Z0-9]{2,4})+$/;
		oEmail.value = oEmail.value.toString().trim() || '';

		if(oEmail.value != ''){
			if(filtro.test(oEmail.value))
				bRet = true; //e-mail valido
			else
				bRet = false; //e-mail no valido
		}

		if(bRet) return false;
		else return(_JaSper.funcs._t('valida/email')); //e-mail no valido
	},

	/**
	 * Validacion de campos de fechas
	 * Comprueba que una fecha introducida es valida y la convierte al formato aaaa-mm-dd hh:mm:ss; no distingue formato europeo de americano, puede dar errores en esa confusion
	 * 
	 * @param object oFecha Fecha a validar
	 * @param string formato Formato de la fecha
	 * @return boolean
	 */
	fechas: function (oFecha, formato){
		if(!formato)
			formato = 'aaaa-mm-dd hh:mm:ss';

		var bRet = false; //fecha incorrecta
		oFecha.value = oFecha.value.toString().trim() || '';

		if(oFecha.value != ''){
			var fechaInt = Date.parse(oFecha.value.replace(/-/g, '/'));
			if(!isNaN(fechaInt)){
				var fecha = new Date();
				fecha.setTime(fechaInt);
				if(fecha != 'Invalid Date'){
					bRet = true; //fecha correcta
				}
			}
		}

		if(bRet) return false;
		else return(_JaSper.funcs._t('valida/fechas') + formato); //fecha incorrecta
	},

	/**
	 * Validacion de campos fichero, comprueba la extension del archivo entre las permitidas
	 * 
	 * @param object oFichero Fichero a validar
	 * @param string extensiones Extensiones permitidas
	 * @return boolean
	 */
	fichero: function (oFichero, extensiones){
		var bRet = true;

		oFichero.value = oFichero.value.toString().trim() || '';
		if(oFichero.value != ''){
			var aTemp = new Array();
			aTemp = oFichero.value.split('.');

			if(extensiones.indexOf(aTemp[aTemp.length - 1].toLowerCase()) >= 0)
				bRet = true; //tipo de fichero correcto
			else
				bRet = false; //tipo de fichero no permitido
		}

		if(bRet) return false;
		else return(_JaSper.funcs._t('valida/fichero')); //tipo de fichero no permitido
	},

	/**
	 * validador IBAN (International Bank Account Number)
	 * incluido el formato de cuenta especifico de cada pais
	 *
	 * @see https://github.com/jzaefferer/jquery-validation/blob/master/src/additional/iban.js
	 * @param object oIban Codigo IBAN a validar
	 * @return boolean
	 */
	iban: function(oIban){
		var bRet = true;

		oIban.value = oIban.value.toString().trim() || '';
		if(oIban.value != ''){
			// remove spaces and to upper case
			var iban = oIban.value.replace(/[^a-zA-Z0-9]/img, "").toUpperCase(),
				ibancheckdigits = "",
				leadingZeroes = true,
				cRest = "",
				cOperator = "",
				ibancheck, charAt, cChar, ibanregexp, i, p;

			// check the country code and find the country specific format
			var countrycode = iban.substring(0, 2);
			var bbancountrypatterns = {
				"AD": "\\d{8}[\\dA-Z]{12}", "AE": "\\d{3}\\d{16}", "AL": "\\d{8}[\\dA-Z]{16}", "AT": "\\d{16}", "AZ": "[\\dA-Z]{4}\\d{20}",
				"BA": "\\d{16}", "BE": "\\d{12}", "BG": "[A-Z]{4}\\d{6}[\\dA-Z]{8}", "BH": "[A-Z]{4}[\\dA-Z]{14}", "BR": "\\d{23}[A-Z][\\dA-Z]",
				"CH": "\\d{5}[\\dA-Z]{12}", "CR": "\\d{17}", "CY": "\\d{8}[\\dA-Z]{16}", "CZ": "\\d{20}",
				"DE": "\\d{18}", "DK": "\\d{14}", "DO": "[A-Z]{4}\\d{20}",
				"EE": "\\d{16}", "ES": "\\d{20}",
				"FI": "\\d{14}", "FO": "\\d{14}", "FR": "\\d{10}[\\dA-Z]{11}\\d{2}",
				"GB": "[A-Z]{4}\\d{14}", "GE": "[\\dA-Z]{2}\\d{16}", "GI": "[A-Z]{4}[\\dA-Z]{15}", "GL": "\\d{14}", "GR": "\\d{7}[\\dA-Z]{16}", "GT": "[\\dA-Z]{4}[\\dA-Z]{20}",
				"HR": "\\d{17}", "HU": "\\d{24}",
				"IE": "[\\dA-Z]{4}\\d{14}", "IL": "\\d{19}", "IS": "\\d{22}", "IT": "[A-Z]\\d{10}[\\dA-Z]{12}",
				"KW": "[A-Z]{4}[\\dA-Z]{22}", "KZ": "\\d{3}[\\dA-Z]{13}",
				"LB": "\\d{4}[\\dA-Z]{20}", "LI": "\\d{5}[\\dA-Z]{12}", "LT": "\\d{16}", "LU": "\\d{3}[\\dA-Z]{13}", "LV": "[A-Z]{4}[\\dA-Z]{13}",
				"MC": "\\d{10}[\\dA-Z]{11}\\d{2}", "MD": "[\\dA-Z]{2}\\d{18}", "ME": "\\d{18}", "MK": "\\d{3}[\\dA-Z]{10}\\d{2}", "MR": "\\d{23}", "MT": "[A-Z]{4}\\d{5}[\\dA-Z]{18}", "MU": "[A-Z]{4}\\d{19}[A-Z]{3}",
				"NL": "[A-Z]{4}\\d{10}", "NO": "\\d{11}",
				"PK": "[\\dA-Z]{4}\\d{16}", "PL": "\\d{24}", "PS": "[\\dA-Z]{4}\\d{21}", "PT": "\\d{21}",
				"RO": "[A-Z]{4}[\\dA-Z]{16}", "RS": "\\d{18}",
				"SA": "\\d{2}[\\dA-Z]{18}", "SE": "\\d{20}", "SI": "\\d{15}", "SK": "\\d{20}", "SM": "[A-Z]\\d{10}[\\dA-Z]{12}",
				"TN": "\\d{20}", "TR": "\\d{5}[\\dA-Z]{17}",
				"VG": "[\\dA-Z]{4}\\d{16}"
			};

			var bbanpattern = bbancountrypatterns[countrycode];

			if(typeof bbanpattern === "undefined") //pais desconocido
				bRet = false;

			ibanregexp = new RegExp("^[A-Z]{2}\\d{2}" + bbanpattern + "$", "");
			if(!(ibanregexp.test(iban))){
				bRet = false; // invalid country specific format
			}

			// now check the checksum, first convert to digits
			ibancheck = iban.substring(4, iban.length) + iban.substring(0, 4);
			for(i = 0; i < ibancheck.length; i++){
				charAt = ibancheck.charAt(i);
				if(charAt !== "0"){
					leadingZeroes = false;
				}
				if(!leadingZeroes){
					ibancheckdigits += "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ".indexOf(charAt);
				}
			}

			// calculate the result of: ibancheckdigits % 97
			for(p = 0; p < ibancheckdigits.length; p++){
				cChar = ibancheckdigits.charAt(p);
				cOperator = "" + cRest + "" + cChar;
				cRest = cOperator % 97;
			}
			bRet = (cRest === 1);
		}

		if(bRet) return false;
		else return(_JaSper.funcs._t('valida/iban'));
	},

	/**
	 * Validacion de campos NIF/NIE
	 * NIF sin espacios, 10 caracteres (NIE) maximo
	 *
	 * @param object oNif NIF a validar
	 * @return boolean
	 */
	nif: function (oNif){
		var bRet = true, mensaje = '';

		oNif.value = (oNif.value.toString().trim() || '').toUpperCase();
		if(oNif.value != ''){
			dni = oNif.value.substr(0, oNif.value.length-1); //NIF
			if(oNif.value.charAt(0) == 'X')
				dni = dni.substr(1); //NIE

			letra = oNif.value.charAt(sNif.length-1);

			if(dni.length == 8 && isNaN(letra)){
				var control = 'TRWAGMYFPDXBNJZSQVHLCKE';
				pos = dni % 23;
				control = control.charAt(pos);

				if(control==letra)
					bRet = true; //nif correcto
			}
			else{
				if(oNif.value.charAt(0) == 'X')
					mensaje += _JaSper.funcs._t('valida/nif1'); //NIE
				else
					mensaje += _JaSper.funcs._t('valida/nif2'); //NIF

				bRet = false; //nif incorrecto
			}
		}

		if(bRet) return false;
		else return mensaje;
	},

	/**
	 * Validacion de numericos, con rango entre "menor" y "mayor" (opcionales ambos)
	 *
	 * @param object oNumero Objeto a evaluar
	 * @param integer menor Rango minimo
	 * @param integer mayor Rango máximo
	 * @return boolean
	 */
	numeros: function (oNumero, menor, mayor){
		var bRet = true;

		oNumero.value = oNumero.value.toString().trim() || '';
		if(oNumero.value != ''){
			if(typeof menor == 'undefined')
				menor = 0;
			if(typeof mayor == 'undefined')
				mayor = Math.pow(10, oNumero.maxLength);

			var num = parseInt(oNumero.value);

			if((num >= menor && num <= mayor) || oNumero == '')
				bRet = true;
			else
				bRet = false;
		}

		if(bRet) return false;
		else return(_JaSper.funcs._t('valida/numeros1') + menor + _JaSper.funcs._t('valida/numeros2') + mayor); //numero incorrecto
	},

	/**
	 * Campo de caracter obligatorio, no puede tener valor nulo
	 * Se llama en el envio del formulario
	 *
	 * @param object oCampo Campo a validar
	 * @return boolean
	 */
	obligatorio: function (oCampo){
		var bRet = true, text = '';

		oCampo.value = oCampo.value.toString().trim() || '';
		if(oCampo.value != '') bRet = true;
		else{
			text = JaSper('<label>', oCampo.parentNode).text(); //title for error container
			bRet = false;
		}

		if(bRet) return false;
		else return('"' + text + '"' + _JaSper.funcs._t('valida/obligatorio'));
	},

	/*version para radio buttons*/
	obligatorioRadio: function (oRadio){
		var bRet = true;

		var cnt = -1, objName = oRadio;
		for(var i=objName.length-1; i > -1; i--){
			if(objName[i].checked){
				cnt = i; i = -1;
			}
		}

		if(cnt > -1) bRet = true;
		else{
			text = JaSper('<label>', oRadio.parentNode).text(); //title for error container
			bRet = false;
		}

		if(bRet) return false;
		else return('"' + text + '"' + _JaSper.funcs._t('valida/obligatorioRadio'));
	},

	/**
	 * Fuerza la entrada de solo numeros barra (/) y guion (-)
	 * Llamar con: onkeypress="return JaSper(this).valTeclasFechas(event);"
	 * o 'JaSper('#objId').addEvent("keypress", function (e){this.valTeclasFechas(e);});'
	 * 
	 * @param object oCampo Campo a limitar
	 * @param event ev Event
	 * @return boolean
	 */
	teclasFechas: function (oCampo, ev){
		var bRet = true;
		var ev = ev || window.event;

		var char_code = JaSper.event.keyCode(ev);
		//permite la entrada de numeros, espacio, dos puntos, barra y guion
		if(charCode > 31 && (charCode < 48 || charCode > 58) && charCode != 32 && charCode != 47 && charCode != 45){
			bRet = false; //alert("No son caracteres de fecha");
			_JaSper.event.preventDefault(ev); //evita la pulsacion
		}
		else{
			bRet = true;
			if(oCampo.value.indexOf(' ') != -1 && charCode == 32)
				bRet = false;
		}

		return(!bRet);
	},

	/**
	 * Fuerza la entrada de solo numeros y coma decimal
	 * Llamar con: onkeypress="return valTeclasNumeros(event);"
	 * o 'JaSper('#objId').addEvent("keypress", function (e){this.valTeclasNumeros(e);});'
	 * 
	 * @param object oCampo Campo a limitar
	 * @param event ev Evento
	 * @param boolean decimal true\~spanish permite punto decimal\~english allow decimal point\~
	 * @return boolean
	 */
	teclasNumeros: function (oCampo, ev, decimal){
		var bRet = true;
		var ev = ev || window.event;
		var decimal = typeof(decimal) != 'undefined' ? decimal : true;

		var char_code = _JaSper.event.keyCode(ev);
		if(decimal){ //permite la entrada de numeros y punto decimal
			if(char_code > 31 && (char_code < 48 || char_code > 57) && char_code != 46){
				bRet = false; //alert("no es un numero")
				_JaSper.event.preventDefault(ev); //evita la pulsacion
			}
			else{
				bRet = true;
				if(oCampo.value.indexOf('.') != -1 && char_code == 46) bRet = false;
			}
		}
		else{ //permite la entrada de numeros sin punto decimal
			if(char_code > 31 && (char_code < 48 || char_code > 57)){
				bRet = false; //alert("no es un numero")
				_JaSper.event.preventDefault(ev); //evita la pulsacion
			}
			else
				bRet = true;
		}

		return(!bRet);
	},

	/**
	 * \~spanish validacion de campos URL
	 * \~english URL validation\~
	 * 
	 * @param object oUrl Objeto a validar
	 * @return boolean
	 */
	url: function (oUrl){
		var bRet = true;
		var filtro = /(((ht|f)tp(s?):\/\/)|(www\.[^ [\]()\n\r\t]+)|(([012]?[0-9]{1,2}\.){3}[012]?[0-9]{1,2})\/)([^ [\](),;"'<>\n\r\t]+)([^. [\](),;"'<>\n\r\t])|(([012]?[0-9]{1,2}\.){3}[012]?[0-9]{1,2})/;
		//var filtro = /((ht|f)tp(s?):\/\/)([^ [\](),;"'<>\n\r\t]+)([^. [\](),;"'<>\n\r\t])/; //fuerza a que haya "http://" (o lo que corresponda) al principio

		oUrl.value = oUrl.value.toString().trim() || '';
		if(oUrl.value != ''){
			if(filtro.test(this.nodes[i].value))
				bRet = true; //url valida
			else
				bRet = false; //url no valida
		}

		if(bRet) return false;
		else return(_JaSper.funcs._t('valida/url'));
	}

});
