<?php
# Copyright (C) 2011 José M. Carnero <jm_carnero@sargazos.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
# http://www.gnu.org/copyleft/gpl.html

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta
defined('CLASSES_DIR') or define('CLASSES_DIR', dirname(__FILE__));

/**
 * Clase de traducción de mensajes
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package L10n
 */

/**
 * Instanciador (singleton) de la clase de traducciones
 *
 * @param string $sLang Lengua en la que se recuperan las traducciones (false detecta el lenguaje del navegador)
 * @param string $sClass Clase que solicita traducciones
 * @param string $sClassDatos Sufijo de la clase de recuperación de datos (por ahora: csv, ddbb)
 * @param string $sDirTrads Directorio donde encontrar las traducciones ('l10n' por defecto)
 * @param array $aDatosConsulta Parametros para la clase de recuperación de datos
 * @param boolean $bVerTiempos Ver los tiempos empleados
 */
function &l10n_sel($sLang = false, $sClass = false, $sClassDatos = 'csv', $sDirTrads = false, $aDatosConsulta = array(), $bVerTiempos = false){
	static $oObj = null;

	if($oObj === null){
		require_once(CLASSES_DIR.'/class.redatazo.inc'); //sistema de adquisicion de datos
		if(!class_exists('Redatazo')){
			//die('No se puede usar el tipo de adquisición de datos -> redatazo' . "\n");
			throw new Exception('No se puede usar el tipo de adquisición de datos -> redatazo');
			return false;
		}

		if(!$sDirTrads){
			$sDirTrads = CLASSES_DIR.'/l10n/';
		}
		//$aDatosConsulta = array('campos' => 'clave, valor', 'filaTitulos' => 0, 'separadorCampos' => '|', 'cercoCampos' => '', 'fichero' => CLASSES_DIR.'/l10n/es.l10n');
		$aDatosConsulta = array('campos' => 'clave, valor', 'filaTitulos' => 0, 'separadorCampos' => '|');
		$oObj = new L10n($sClass, $sClassDatos, $sDirTrads, $aDatosConsulta, $sLang);
	}
	elseif(!empty($sLang) || !empty($sClass) || !empty($sDirTrads) || !empty($aDatosConsulta)){
		$oObj->l10n_ini($sClass, $sClassDatos, $sDirTrads, $aDatosConsulta, $sLang);
	}

	$oObj->setVerTiempos($bVerTiempos);

	/*$aError = $obj->getError();
	if(DEBUG && !empty($aError)){
		var_dump('l10n_sel:', $aError);
	}/**/

	return $oObj;
}

/**
 * Pone "locales" al idioma seleccionado
 *
 * @param string $idm Idioma con el que se pondran los "locales" (al idioma automaticamente seleccionado si no se pasa ninguno)
 * @return array
 */
function _locale($idm = ''){
	$oLoc = l10n_sel(); //objeto traductor

	return $oLoc->setLocale($idm);
}

/**
 * Devuelve (echo/return) el termino "traducido"
 *
 * @param numero de parametros variable, el primero es la clave a buscar y el resto los argumentos (%s) que precise
 * @return string
 */
function _trad(){
	$aArgumentos = func_get_args();
	//$sClave = $aArgumentos[0];

	$oTrad = l10n_sel(); //objeto traductor

	echo call_user_func_array(array($oTrad, 'getTrad'), $aArgumentos);
}

/**
 * Devuelve (return) el termino "traducido"
 *
 * @param numero de parametros variable, el primero es la clave a buscar y el resto los argumentos (%s) que precise
 * @return array
 */
function _tradR(){
	$aArgumentos = func_get_args();
	//$sClave = $aArgumentos[0];

	$oTrad = l10n_sel(); //objeto traductor

	return call_user_func_array(array($oTrad, 'getTrad'), $aArgumentos);
}

/**
 * Guarda las traducciones que no encuentre en los ficheros (o bases de datos) de traduccion, como: huerfana|huerfana
 *
 * @return void
 */
function _tradGuardaHuerfanas(){
	$oTrad = l10n_sel(); //objeto traductor
	$oTrad->setGuardaHuerfanas(true);
	//if($oTrad->getError()) var_dump($oTrad->getError());
}

/**
 * Clase para traducción de textos.
 * Por defecto los textos se buscan en el directorio (al mismo nivel que se encuentre este archivo) "l10n", con el nombre: "nombre-de-la-clase-es_ES.l10n" (siendo "es_ES" el codigo del idioma que interese, formato ISO 3166)
 *
 * #core PHP 4
 * Ejemplo de uso:
 * <code>
include_once("./class.l10n.inc");
l10n_sel();
_locale();
_trad('clave_a_traducir');
//var_dump('uso de traducciones:', l10n_sel()->getTraduccionUso());
 * </code>
 *
 * ejemplo de un fichero de traducciones (clave|traduccion)
 * <code>
## ejemplo de comentario
clave|clave traducida
otra clave|otra clave traducida
 * </code>
 *
 * @todo convertir en estatica? se irian añadiendo los mensajes de las clases que se instanciasen (sin repeticion) y guardando los mensajes de cada instancia (como identificarlas)
 * @todo posibilidad de guardar registro historico, asi como uno de claves de traduccion a efectos estadisticos
 * @todo releer los ficheros de traduccion cada vez que se cambien el idioma o mantener las traducciones viejas y simplemente cargas las nuevas en el nuevo idioma?
 * @since 2011-03-25
 */
class L10n {
	/**
	 * Clase que solicita traducciones
	 *
	 * @access private
	 * @var string
	 */
	var $sClass;

	/**
	 * Sufijo de la clase de recuperación de datos
	 *
	 * @access private
	 * @var string
	 */
	var $sClassDatos;

	/**
	 * Parametros para la clase de recuperación de datos
	 *
	 * @access private
	 * @var array
	 * @see l10n::aDatosConsulta()
	 */
	var $aDatosConsulta;

	/**
	 * Objeto de recuperación de datos
	 *
	 * @access private
	 * @var object
	 * @see l10n::recuperaDatos()
	 */
	var $oDatos;

	/**
	 * Directorio donde encontrar las traducciones ("l10n" bajo el actual, por defecto)
	 *
	 * @access private
	 * @var object
	 * @see l10n::setDirTrads($dir)
	 */
	var $sDirTrads;

	/**
	 * Errores sucedidos
	 *
	 * @access private
	 * @var array
	 * @see l10n::getError()
	 */
	var $aError;

	/**
	 * Mensajes de error la clase, a fin de que puedan ser traducidos/modificados; las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas, si es necesario poner un % se pondra como %%
	 *
	 * @access private
	 * @var array
	 * @see l10n::getErrorMensaje($clave), l10n::setErrorMensaje($clave, $valor)
	 */
	var $aErrorMensajes;

	/**
	 * Si true pone en los ficheros de traduccion las que no se encuentren en el
	 *
	 * @access public
	 * @var boolean
	 * @see l10n::setGuardaHuerfanas($bGuarda)
	 */
	var $bGuardaHuerfanas;

	/**
	 * Lengua en la que se recuperan los textos, si no se asigna ninguna comprueba cual es la que acepta el navegador
	 *
	 * @access private
	 * @var string
	 * @see l10n::setLang($lang),l10n::getLang()
	 */
	var $aLang;

	/**
	 * Lengua por defecto para cuando no se puede recuperar automaticamente la lista de lenguajes del navegador (como usando file_get_contents())
	 *
	 * @access public
	 * @var string
	 */
	var $aLangDefecto;

	/**
	 * Array con los textos en el idioma solicitado (traducciones)
	 *
	 * @access private
	 * @var array
	 * @see l10n::getTrad()
	 */
	var $aTraduccion;

	/**
	 * Devuelve el numero de usos de cada traduccion
	 * -1 para traducciones inexistentes
	 *
	 * @access private
	 * @var array
	 * @see l10n::getTraduccionUso()
	 */
	var $aTraduccionUso;

	/**
	 * Devuelve los tiempos de las traducciones y mensajes de debug (en la misma linea, al principio)
	 *
	 * @access private
	 * @var integer
	 * @see l10n::setVerTiempos($ver)
	 */
	var $bVerTiempos;

	/**
	 * Muestra mensajes de depuracion
	 *
	 * @var boolean
	 * @see listado::debug(),listado::setDebug($debug)
	 */
	var $debug = false;

	/**
	 * Comprueba los datos para la consulta
	 *
	 * @param array $datosconsulta Datos que requiera la consulta
	 * @return float
	 */
	private function datosConsulta($datosconsulta = array()){
		$this->debug();

		/*if(empty($this->aDatosConsulta)){
			$this->aDatosConsulta = array('campos' => 'clave, valor',
			'filaTitulos' => 0,
			//'separadorCampos' => ',',
			//'cercoCampos' => '"',
			'separadorCampos' => '|',
			'cercoCampos' => '',
			'fichero' => $this->sDirTrads.$this->sClass.'-'.$this->aLang[0].'.err',
			);
		}*/
		if($this->debug){
			var_dump(array('{$this->datosConsulta()} $this->aDatosConsulta [antes]: ' => $this->aDatosConsulta));
		}

		if(!$this->instCount()){
			$this->aDatosConsulta['campos'] = empty($datosconsulta['campos']) ? (empty($this->aDatosConsulta['campos']) ? 'clave, valor' : $this->aDatosConsulta['campos']) : $datosconsulta['campos'];
			$this->aDatosConsulta['filaTitulos'] = empty($datosconsulta['filaTitulos']) ? (empty($this->aDatosConsulta['filaTitulos']) ? 0 : $this->aDatosConsulta['filaTitulos']) : $datosconsulta['filaTitulos'];
			$this->aDatosConsulta['separadorCampos'] = empty($datosconsulta['separadorCampos']) ? (empty($this->aDatosConsulta['separadorCampos']) ? '|' : $this->aDatosConsulta['separadorCampos']) : $datosconsulta['separadorCampos'];

			//if(empty($datosconsulta['sFichero'])) $this->aDatosConsulta['sFichero'] = empty($this->aDatosConsulta['sFichero'])?$this->sDirTrads.$this->aLang[0].'.l10n':$this->aDatosConsulta['sFichero'];
			if(empty($datosconsulta['fichero'])){
				$this->aDatosConsulta['fichero'] = $this->sDirTrads . $this->aLang[0] .'.l10n';
			}
			else{
				if(is_readable($datosconsulta['fichero'])){
					$this->aDatosConsulta['fichero'] = $datosconsulta['fichero'];
				}
				elseif(is_readable(dirname(__FILE__) . '/' . ltrim($datosconsulta['fichero'], ' /\\'))){
					$this->aDatosConsulta['fichero'] = dirname(__FILE__) . '/' . ltrim($datosconsulta['fichero'], ' /\\');
				}
				else{
					$this->setError(__METHOD__ . '_1', $datosconsulta['fichero']);
					return false;
				}
			}
		}

		if($this->debug){
			var_dump(array('{$this->datosConsulta()} $this->aDatosConsulta [despues]: ' => $this->aDatosConsulta, $this->sDirTrads));
		}

		return true;
	}

	/**
	 * Muestra informacion de depuracion
	 *
	 * @since 2012-02-10
	 */
	private function debug(){
		if(!$this->debug) return false;
		$backtrace = debug_backtrace();

		if(empty($backtrace)) return false;

		$output = '<ul style="border:solid red 1px;">';
		//quitar los pasos con referencias a esta clase/metodos/funciones
		foreach($backtrace as $clave => $bt){
			$args = empty($bt['args']) ? '' : @implode(', ', $bt['args']);

			$bt['class'] = empty($bt['class']) ? '' : $bt['class'];
			$bt['function'] = empty($bt['function']) ? '' : $bt['function'];
			if($bt['function'] == 'debug' || $bt['function'] == 'listado_sel'){
				unset($backtrace[$clave]);
				continue;
			}

			$output .= '<li> call: ';
			$output .= (empty($bt['class']) ? '' : $bt['class']);
			$output .= (empty($bt['type']) ? '' : $bt['type']);
			$output .= (empty($bt['function']) ? '' : $bt['function']) . '(' . $args . ')';
			$output .= ' // file: ' . $bt['file'] . ' (line ' . $bt['line'] . ') ';
			$output .= (method_exists($this, 'microtimeSeg') ? $this->microtimeSeg() : '');
			$output .= '</li>';
		}
		$output .= '</ul>';
		echo($output);
	}

	/**
	 * Devuelve los errores que se hayan producido.
	 *
	 * @return string
	 */
	public function getError(){
		return $this->aError;
	}

	/**
	 * Muestra el texto asignado a un error
	 * Si no se pasa ninguna clave se muestran todos
	 *
	 * @todo mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @param string $clave Clave del mensaje de error
	 * @return string
	 */
	public function getErrorMensaje($clave = false){
		if($clave === false) return $this->aErrorMensajes;
		return $this->aErrorMensajes[$clave];
	}

	/**
	 * Devuelve el codigo de idioma usado actualmente
	 *
	 * @param boolean $codCorto Si true devuelve el formato corto (dos primeros caracteres)
	 * @return string
	 */
	public function getLang($codCorto = false){
		if(empty($this->aLang)) return false;

		if($codCorto) return substr($this->aLang[0], 0, 2); //devuelve formato corto; ej.: "es", del codigo "es_AR"
		else return $this->aLang[0];
	}

	/**
	 * Busca la clave pasada en el array de traducciones y devuelve la traduccion con parametros.
	 *
	 * @param numero de parametros variable, el primero es la clave a buscar y el resto los argumentos (%s) que precise
	 * @return string
	 */
	public function getTrad(){
		$argumentos = func_get_args();
		$clave = array_shift($argumentos);

		$sCadena = $clave; //si no se encuentra traduccion se devuelve la clave pasada
		if(!empty($this->aTraduccion[$clave])){
			//limpia la cadena para evitar problemas con "*printf"
			$sCadena = str_replace('%s', '\s|', str_replace('%%', '\%|', $this->aTraduccion[$clave]));

			//comprueba que el numero de argumentos sean los mismos en el mensaje que los pasados
			$aTemp = explode('\s|', $sCadena);
			if(count($argumentos) != (count($aTemp) - 1)){
				$this->setError(__METHOD__, $clave);
				return false;
			}

			//limpia y recompone la cadena para evitar problemas con "*sprintf"
			$sCadena = str_replace('\%|', '%%', str_replace('\s|', '%s', str_replace('%', '\~|', $sCadena)));
			$sCadena = vsprintf($sCadena, $argumentos);
			if($this->bVerTiempos){
				$sCadena = '[' . $this->microtimeSeg() . '] ' . $sCadena;
			}
			$sCadena = str_replace('\~|', '%', $sCadena);
		}

		if(!isset($this->aTraduccion[$clave])){ //no se encuentra en los archivos de traduccion
			$this->aTraduccionUso[$clave] = -1; //no existe la clave de traduccion

			if($this->bGuardaHuerfanas){
				$this->aTraduccionUso[$clave] = !$this->guardaHuerfanas($clave) ? 1 : -1; //intenta poner clave|clave en los archivos de traduccion
			}
		}
		else{
			$this->aTraduccionUso[$clave] = empty($this->aTraduccionUso[$clave]) ? 1 : $this->aTraduccionUso[$clave] + 1;
		}

		return $sCadena;
	}


	/**
	 * Devuelve el recuento de traducciones usadas.
	 *
	 * @since 2012-02-09
	 * @param integer $filtro = -1 devuelve solo las que no se encuentren en el archivo de traducciones, = n devuelve aquellas que se usen n o mas veces; = false devuelve todas
	 * @return array
	 */
	public function getTraduccionUso($filtro = false){
		$temp = $this->aTraduccionUso;

		if($filtro == -1){
			foreach($temp as $clave => $valor){
				if($filtro != $valor) unset($temp[$clave]);
			}
		}
		if($filtro > 0){
			foreach($temp as $clave => $valor){
				if($filtro > $valor) unset($temp[$clave]);
			}
		}
		ksort($temp);
		return $temp;
	}

	/**
	 * Guarda claves huerfanas en ficheros de traduccion
	 * de momento solo en el fichero de traduccion del idioma activo
	 *
	 * @since 2012-06-22
	 * @param string $sClave Clave huerfana
	 * @return boolean
	 */
	private function guardaHuerfanas($sClave = false){
		if(!$this->bGuardaHuerfanas || empty($sClave)){
			return false;
		}

		if(!is_writable($this->aDatosConsulta['fichero'])){
			$this->setError(__METHOD__ . '_1', $this->aDatosConsulta['fichero']);
			return false;
		}
		if(!$fichero = fopen($this->aDatosConsulta['fichero'], 'a')){
			$this->setError(__METHOD__ . '_2', $this->aDatosConsulta['fichero']);
			return false;
		}
		$sTraduccion = $sClave . $this->aDatosConsulta['separadorCampos'] . $sClave . "\n";
		if(fwrite($fichero, $sTraduccion) === FALSE){
			$this->setError(__METHOD__ . '_3', $sClave, $this->aDatosConsulta['fichero']);
			return false;
		}
		fclose($fichero);

		return true;
	}


	/**
	 * Contador de (intentos de) instancias de la clase;
	 *
	 * @return integer
	 */
	protected function instCount(){
		$this->debug();

		if(!isset($iInstCount)){
			static $iInstCount = 0;
		}
		else $iInstCount++;

		return $iInstCount;
	}

	/**
	 * Constructor
	 *
	 * @param string $sClass Clase que solicita traducciones
	 * @param string $sClassDatos Sufijo de la clase de recuperación de datos
	 * @param string $sDirTrads Directorio donde encontrar las traducciones ('l10n' por defecto)
	 * @param array $aDatosConsulta Parametros para la clase de recuperación de datos
	 * @param string $sLang Lengua en la que se recuperan los textos
	 */
	function L10n($sClass = false, $sClassDatos = false, $sDirTrads = false, $aDatosConsulta = array(), $sLang = false){
		$this->aError = array();
		$this->aErrorMensajes = array();
		$this->setErrorMensaje(); //inicializacion de los mensajes de error

		$this->bGuardaHuerfanas = false;
		$this->aLangDefecto = 'es_es';
		$this->aTraduccion = $this->aTraduccionUso = array();
		$sLang = ($sLang === false) ? '' : $sLang; //esto permite la deteccion automatica de idioma (si se pasa false no asigna lenguaje ni lo busca)

		$this->setVerTiempos();

		$this->l10n_ini($sClass, $sClassDatos, $sDirTrads, $aDatosConsulta, $sLang);
	}

	/**
	 * Inicializador de la clase; no deberia instanciarse directamente, sino con l10n_sel() (al principio de este archivo), que llamara a este o al constructor, segun sea necesario; php4 dificulta patrones singleton ya que no permite propiedades estaticas
	 *
	 * @param string $sClass Clase que solicita errores
	 * @param string $sClassDatos Sufijo de la clase de recuperación de datos
	 * @param string $sDirTrads Directorio donde encontrar las traducciones ('l10n' por defecto)
	 * @param array $aDatosConsulta Parametros para la clase de recuperación de datos
	 * @param string $sLang Lengua en la que se recuperan los mensajes error
	 * @return boolean
	 */
	public function l10n_ini($sClass = false, $sClassDatos = false, $sDirTrads = false, $aDatosConsulta = array(), $sLang = false){
		$this->sClass = empty($sClass) ? '' : $sClass;

		$this->setLang($sLang);

		$this->setDirTrads($sDirTrads);

		if($this->debug){
			var_dump(array('{$this->l10_ini}()} $aDatosConsulta: ' => $aDatosConsulta));
		}

		$this->sClassDatos = $sClassDatos;
		$this->datosConsulta($aDatosConsulta);

		$this->recuperaDatos(); //recupera los datos de errores

		return true;
	}

	/**
	 * Lista de codigos de idioma y locales soportados.
	 * Se puede cambiar para que devuelva uno por defecto en caso de que los del usuario no esten soportados; si el usuario tiene entre su lista de idiomas uno soportado esto no es necesario, ya que al comprobar el directorio de traducciones se toma el primero soportado que tenga fichero valido
	 * De momento solo se usan los dos primeros caracteres; no se contemplan codigos regionales
	 *
	 * @todo gestionar variantes; por ahora solo se gestiona el lenguaje principal "es", no la variante "es_ar"
	 * @param string $sLang Codigo de idioma (ISO 3166) a comprobar
	 * @param boolean $bLocale Asignar (true), o no (false) el locale; si true devolvera el "locale" que se haya asignado o false si no se ha conseguido asignar, si false devolvera el codigo de idioma que corresponda al proporcionado en parametro
	 * @return boolean
	 */
	private function locale($sLang = false, $bLocale = false){
		$this->debug();

		$sLocale = false; //queda con false si no se asigna (o no se consigue asignar) el locale
		$sTemp = false;

		$sLang = ($sLang) ? str_replace('-', '_', strtolower($sLang)) : $sLang;

		switch($sLang){ //TODO revisar grupos de idiomas
			case 'ar': //Arabe
			case 'ar_ae': //Árabe (Emiratos árabes unidos)
			case 'ar_bh': //Árabe (Bahrein)
			case 'ar_dz': //Argelino
			case 'ar_eg': //Egyptien
			case 'ar_iq': //Iraquien
			case 'ar_jo': //Jordanien
			case 'ar_kw': //Koweitien
			case 'ar_lb': //Libanes
			case 'ar_ly': //Libion
			case 'ar_ma': //Marroquí
			case 'ar_om': //Oman
			case 'ar_qa': //Quatar
			case 'ar_sa': //Árabe (Arabia saudí)
			case 'ar_sy': //Sirio
			case 'ar_tn': //Tunecino
			case 'ar_ye': //Yémen
				$sTemp = 'ar';
				break;
			case 'da': //Danés
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'da_DK.ISO-8859-15', 'da_DK.iso885915') : false;
				break;
			case 'de': //Alemán
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'de_DE@euro', 'de_DE', 'de', 'ge','deu_deu') : false;
			case 'de_at': //Alemán (Austria)
			case 'de_ch': //Alemán (Suiza)
			case 'de_li': //Alemán (Liechtenstein)
			case 'de_lu': //Alemán (Luxemburgo)
				$sTemp = 'de';
				break;
			case 'en': //Inglés
				if($bLocale){
					date_default_timezone_set('Europe/London');
					$sLocale = empty($sLocale) ? setlocale(LC_ALL, 'en_EN', 'ENG', 'English') : false;
				}
			case 'en_au': //Australiano
			case 'en_bz': //Inglés (Bélize)
			case 'en_ca': //Canadiense
			case 'en_gb': //Inglés (Gran Bretaña)
			case 'en_ie': //Irlandes
			case 'en_jm': //Jamaicano
			case 'en_nz': //Neocelandes
			case 'en_us': //Inglés (USA)
				if($bLocale){
					date_default_timezone_set('America/New_York');
					$sLocale = empty($sLocale) ? setlocale(LC_ALL, 'en_US', 'en_US.utf8', 'en_US.UTF8', 'en_US.UTF-8', 'en_US.8859-1', 'en_US', 'American') : false;
				}
			case 'en_za': //Inglés (Africa del sur)
				$sTemp = 'en';
				break;
			case 'es': //Español
			case 'es_es': //Español (España)
				//localizacion del entorno
				if($bLocale){
					date_default_timezone_set('Europe/Madrid');
					$sLocale = empty($sLocale) ? setlocale(LC_ALL, 'es_ES', 'es_ES.UTF-8', 'es_ES@euro', 'spanish', 'Spanish_Spain.1252', 'Spanish_Spain.28605') : false;
				}
			case 'es_ar': //Español (Argentina)
			case 'es_bo': //Español (Bolivia)
			case 'es_cl': //Español (Chile)
			case 'es_co': //Español (Colombia)
			case 'es_cr': //Español (Costa Rica)
			case 'es_ec': //Español (Ecuador)
			case 'es_gt': //Español (Guatemala)
			case 'es_hn': //Español (Honduras)
			case 'es_mx': //Español (Méjico)
			case 'es_ni': //Español (Nicaragua)
			case 'es_pa': //Español (Panamá)
			case 'es_pe': //Español (Peru)
			case 'es_pr': //Español (Puerto Rico)
			case 'es_py': //Español (Paraguai)
			case 'es_sv': //Español (El Salvador)
			case 'es_tt': //Español (Trinidad)
			case 'es_uy': //Español (Uruguai)
			case 'es_ve': //Español (Venezuela)
				$sTemp = 'es';
				break;
			case 'fi': //Finlandés
				if($bLocale){
					date_default_timezone_set('Europe/Helsinki');
					$sLocale = empty($sLocale) ? setlocale(LC_ALL, 'fi_FI.UTF-8','fi_FI@euro','fi_FI','finnish') : false;
				}
				break;
			case 'fr': //Français
				if($bLocale){
					date_default_timezone_set('Europe/Paris');
					$sLocale = empty($sLocale) ? setlocale(LC_ALL, 'fr_FR','fr.UTF8', 'fr.UTF-8') : false;
				}
			case 'fr_be': //Français (Belgique)
			case 'fr_ca': //Français (Canada)
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'fr_CA') : false;
			case 'fr_ch': //Français (Suisse)
			case 'fr_fr': //Français
				if($bLocale){
					date_default_timezone_set('Europe/Paris');
					$sLocale = empty($sLocale) ? setlocale(LC_ALL, 'fr_FR', 'fr_FR.UTF8', 'fr_FR.UTF-8') : false;
				}
			case 'fr_lu': //Français (Luxembourg)
				$sTemp = 'fr';
				break;
			case 'in': //Indonesio
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'id_ID.UTF8', 'id_ID.UTF-8', 'id_ID.8859-1', 'id_ID', 'IND.UTF8', 'IND.UTF-8', 'IND.8859-1', 'IND', 'Indonesian.UTF8', 'Indonesian.UTF-8', 'Indonesian.8859-1', 'Indonesian', 'Indonesia', 'id', 'ID', 'en_US.UTF8', 'en_US.UTF-8', 'en_US.8859-1', 'en_US', 'American', 'ENG', 'English') : false;
				break;
			case 'it': //Italiano
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'it_IT') : false;
			case 'it_ch': //Italiano (Suiza)
				$sTemp = 'it';
				break;
			case 'nl': //Holandés
			case 'nl_be': //Belga
				$sTemp = 'nl';
				break;
			case 'pt': //Portugués
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'pt_PT', 'pt_PT.iso-8859-1', 'pt_PT.utf-8', 'portuguese') : false;
			case 'pt_br': //Portugués (Brasil)
				if($bLocale) $sLocale = empty($sLocale) ? setlocale(LC_ALL, 'pt_BR', 'pt_BR.iso-8859-1', 'pt_BR.utf-8', 'portuguese', 'pt_PT') : false;
				$sTemp = 'pt';
				break;
			case 'ro': //Roumain (Moldavie)
			case 'ro_mo': //Roumain (Moldavia)
				$sTemp = 'ro';
				break;
			case 'ru': //Ruso
			case 'ru_mo': //Ruso (Moldavia)
				$sTemp = 'ru';
				break;
			case 'sv': //Suèdois
			case 'sv_fi': //Suèdois (Finlande)
				$sTemp = 'sv';
				break;
			case 'zh': //Chino
			case 'zh_cn': //Chino (República popular China)
			case 'zh_hk': //Chino (Hong-Kong)
			case 'zh_sg': //Chino (Singapur)
			case 'zh_tw': //Chino (Taïwan)
				$sTemp = 'zh';
				break;
			case 'af': //Africain
			case 'be': //Bieloruso
			case 'bg': //Búlgaro
			case 'ca': //Catalán
			case 'cs': //Cretense
			case 'el': //Griego
			case 'et': //Estonio
			case 'eu': //Vasco
			case 'fa': //Iraní
			case 'fo': //Feroes
			case 'gd': //Galicien
			case 'he': //Hebreo
			case 'hi': //Indú
			case 'hr': //Croata
			case 'hu': //Hungaro
			case 'is': //Islandés
			case 'ja': //Japonés
			case 'ji': //Yiddish
			case 'ko': //Coreano
			case 'lt': //Lituano
			case 'lv': //Lettische
			case 'mk': //Macedonio
			case 'ms': //Malésien
			case 'mt': //Maltés
			case 'no': //Noruego (bokmal)
			//case 'no': //Noruego (Nynorsk)
			case 'pl': //Polaco
			case 'rm': //Rhaeto-Romanic
			case 'sb': //Sorbian
			case 'sk': //Eslovaco
			case 'sl': //Eslovenio
			case 'sq': //Albanes
			//case 'sr': //Serbio (Cyrillic)
			case 'sr': //Serbio (Latin)
			case 'sx': //Estonio
			case 'th': //Thaïlandais
			case 'tn': //Tswana (Afrique du sud)
			case 'tr': //Turco
			case 'ts': //Tsonga (Afrique du sud)
			case 'uk': //Ucraniano
			case 'ur': //Urdu
			case 'vi': //Vietnamita
			case 'xh': //Xhosa (Afrique)
			case 'zu': //Zulú (Afrique)
				$sTemp = $sLang;
				break;
			default:
				//idioma que se usara en caso de no estar soportado el proporcionado
		}

		if($bLocale){
			return $sLocale;
		}
		//return($sTemp.'_'.strtoupper($sTemp));
		return $sTemp;
	}

	/**
	 * Devuelve microtime en segundos
	 *
	 * @param boolean $bFloat Si true devuelve un numero flotante, si false devuelve formateado "legible para humanos" (parecido a ISO 8601)
	 * @return float
	 */
	private function microtimeSeg($bFloat = false){
		list($mseg, $seg) = explode(" ", microtime());

		$fRet = (date('Y-m-d H:i:s', $seg) . substr($mseg, 1));

		if($bFloat){
			$fRet = ((float)$mseg + (float)$seg);
		}

		return $fRet;
	}

	/**
	 * Crea instancia de la clase de recuperacion de datos y recupera los datos de traducciones
	 *
	 * @return float
	 */
	private function recuperaDatos(){
		$this->debug();

		$this->oDatos = new Redatazo(array('motor' => $this->sClassDatos, 'filaTitulos' => $this->aDatosConsulta['filaTitulos'], 'separadorCampos' => $this->aDatosConsulta['separadorCampos']));

		$this->oDatos->select($this->aDatosConsulta['campos']);
		$this->oDatos->from($this->aDatosConsulta['fichero']);

		//resultados
		$this->oDatos->consulta();
		$aTemp = $this->oDatos->getFilas();

		if($aTemp === false){
			$this->setError(__METHOD__ . '_1', $this->oDatos->getError());
		}

		if(empty($aTemp)) return false;

		foreach($aTemp as $valor){
			$valor = array_values($valor); //evita problemas con nombres de columas personalizados

			if(strpos($valor[0], '##') === 0){
				continue; //ignora los comentarios (cualquier linea que empiece con ##)
			}
			elseif((empty($valor[0]) && !empty($valor[1])) || (!empty($valor[0]) && empty($valor[1]))){ //error, clave sin valor o valor sin clave
				$this->setError(__METHOD__ . '_2', (isset($valor[0]) ? $valor[0] : ''), (isset($valor[1]) ? $valor[1] : ''));
				//return false;
			}

			if(!empty($valor[0]) && !empty($valor[1])){
				$this->aTraduccion[$valor[0]] = $valor[1]; //ignora las lineas vacias
			}
		}

		return true;
	}


	/**
	 * Asigna el nivel de debug (false inactivo, true activo). Devuelve el actual nivel de debug
	 *
	 * @since 2012-02-10
	 * @return boolean
	 */
	public function setDebug($debug){
		$this->debug = empty($debug) ? false : true;

		return $this->debug;
	}

	/**
	 * Asigna el directorio donde encontrar los ficheros de traduccion y comprueba si existe el fichero de traduccion (alguno de ellos).
	 *
	 * @param string $sDir Directorio donde encontrar las traducciones
	 * @return boolean
	 */
	public function setDirTrads($sDir = false){
		if($sDir === false){
			return false;
		}

		if(realpath($sDir) === false){
			$sDir = realpath(dirname(__FILE__));
		}
		$sDir = rtrim($sDir, ' /\\') . '/';

		$ret = array();

		if(!($rDir = @opendir($sDir))){
			$this->sDirTrads = false;
			$this->setError(__METHOD__ . '_1', $sDir);
			return false;
		}

		while(($file = readdir($rDir)) !== false){
			foreach($this->aLang as $clave => $valor){
				if(is_readable($sDir . $file) && strpos($file, $valor) !== false){ //idioma válido //TODO comprobar que se ajusta al tipo de fichero buscado (errores, traducciones, ...)
					$ret[$clave] = $valor;
					//break;
				}
			}
		}
		closedir($rDir);
		$this->sDirTrads = $sDir;

		if(empty($ret)){
			$this->setError(__METHOD__ . '_2', $sDir, implode(',', $this->aLang));
			return false;
		}

		$this->aLang = $ret; //confirma de entre los lenguajes detectados cuales tienen ficheros de traduccion

		return true;
	}

	/**
	 * Guarda los errores que se hayan producido.
	 *
	 * @return string
	 */
	private function setError(){
		$aArgs = func_get_args();
		if(empty($aArgs)){
			return false;
		}
		$clave = array_shift($aArgs);

		$this->aError[] = '[' . $this->microtimeSeg() . '] ' . vsprintf($this->aErrorMensajes[$clave], $aArgs);
		//trigger_error($this->aError[count($this->aError)-1]);

		return true;
	}

	/**
	 * Cambia mensajes de error (su texto)
	 * Util, por ejemplo, para traducciones
	 * Si clave igual a false se reinicializan los mensajes
	 *
	 * @param mixed $sClave Clave del mensaje de error, o un array con todos los mensajes de error (array('clave' => 'mensaje de error',[...]);); si no existe se genera un error
	 * @param mixed $sValor Nuevo texto para el mensaje de error
	 * @return boolean
	 */
	public function setErrorMensaje($sClave = false, $sValor = false){
		$aMensajes = array();

		if($sClave === false){ //reinicio de mensajes
			//TODO definir constantes de tipos de error para usar trigger_error() ademas de esto
			//TODO se pierden los mensajes iniciales de las clases herederas
			$aMensajes = array(
			'L10n::datosConsulta_1' => 'Fichero de datos "%s" no accesible, comprueba que la ruta y nombre de archivo son correctos.',
			'L10n::getTrad' => 'Número de parámetros incorrecto para <em>"%s"</em>',
			'L10n::guardaHuerfanas_1' => 'Necesita permisos de escritura en <em>"%s"</em>',
			'L10n::guardaHuerfanas_2' => 'No se ha podido abrir <em>"%s"</em>',
			'L10n::guardaHuerfanas_3' => 'No se ha podido escribir <em>"%s"</em> en el fichero <em>"%s"</em>',
			'L10n::recuperaDatos_1' => 'No se han podido recuperar los datos, error: %s',
			'L10n::recuperaDatos_2' => 'Clave o contenido vacio: "%s", "%s"',
			'L10n::setDirTrads_1' => 'Error al acceder al directorio "%s"',
			'L10n::setDirTrads_2' => 'No se encuentran ficheros de traducción válidos en el directorio "%s", para "%s"',
			'L10n::setLang_1' => 'Lenguaje solicitado ("%s") no soportado.',
			'L10n::setTrad' => 'Mensaje de error mal formateado, o número de variables incorrecto; en: "%s"'
			);
			//return true;
		}
		elseif(is_array($sClave)){
			$aMensajes = $sClave;
		}
		else{
			$aMensajes[$sClave] = $sValor;
		}

		foreach($aMensajes as $c => $v){
			$this->aErrorMensajes[$c] = '{' . $c . '} ' . $v;
		}

		return true;
	}

	/**
	 * Si true pone en los ficheros de traduccion las que no se encuentren en el
	 *
	 * @param boolean $bGuarda True para guardar en los ficheros de traduccion, false para no hacerlo
	 * @return boolean
	 */
	public function setGuardaHuerfanas($bGuarda = false){
		$this->bGuardaHuerfanas = (boolean) $bGuarda;

		return $this->bGuardaHuerfanas;
	}

	/**
	 * Asigna o busca los codigos de lenguaje que se usaran
	 *
	 * @param string $sLang Codigo de lenguaje forzado, o (false) buscar los que usa el navegador del usuario
	 * @return boolean
	 */
	public function setLang($sLang = false){
		if($sLang === false){
			return false;
		}

		$aIdm = array($this->locale($sLang));

		//deteccion automatica de idioma
		if(empty($sLang)){
			$aIdm = array();

			//pruebas con servidor REST devuelven la cadena 'null' como lenguaje
			$sAcceptLanguage = empty($_SERVER["HTTP_ACCEPT_LANGUAGE"]) || $_SERVER["HTTP_ACCEPT_LANGUAGE"] === 'null' ? $this->aLangDefecto : $_SERVER["HTTP_ACCEPT_LANGUAGE"]; //fuerza un lenguaje por defecto si no detecta ninguno (file_get_contents() no envia cabecera con codigos de lenguaje)

			preg_match_all('/(\w{2}(-\w{2})*)(;q=(\d\.\d))*/i', $sAcceptLanguage, $aRes, PREG_SET_ORDER);
			foreach($aRes as $clave => $valor){
				$temp = strtolower($valor[1]); //$valor[4] guarda el orden de importancia de la lista de idiomas del usuario (si valor esta vacio es la mayor importancia)

				$aIdm[] = $this->locale($temp);
			}
		}

		$this->aLang = array_unique($aIdm);
		$_SESSION['l10n'] = substr($this->aLang[0], 0, 2);

		if($aIdm[0] == false){
			$this->setError(__METHOD__ . '_1', $sLang);
			return false;
		}

		return true;
	}

	/**
	 * Asigna "locale" al primer idioma valido
	 *
	 * @param string $sIdm Codigo de idioma (ISO 3166) con el que se pondran los "locales" (al idioma automaticamente seleccionado si no se pasa ninguno)
	 * @return boolean
	 */
	public function setLocale($sIdm = ''){
		$sIdm = empty($sIdm) ? $this->aLang[0] : $sIdm;

		return $this->locale($sIdm, true);
	}

	/**
	 * Pone los tiempos de cada traduccion, mensaje de debug, etc
	 * Al principio de cada linea
	 *
	 * @param string $bVer Si true muestra los tiempos
	 * @return boolean
	 */
	public function setVerTiempos($bVer = false){
		$this->bVerTiempos = $bVer;
		return true;
	}

}
