<?php
# Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>
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

//include_once("class.sqlazo.inc");
//include("class.thumbnail.inc");

/**
 * Clase procesadora de formularios HTML
 * Tanto GET como POST
 *
 * Ejemplo de uso:
 * \code{.php}
if(!empty($_POST['frmFormName'])){
	include_once("inc/class.formazo_proc.inc");

	$oProcFormulario = new formazo_proc();

	$oProcFormulario->setAccionForm('upd');
	$oProcFormulario->setFuenteDatos('tablaSQL');
	$oProcFormulario->setClavePrimaria('id', 'valor');

	if($oProcFormulario->procesarFormulario($_POST['frmFormName'])){ //no hay problemas, procesar la SQL
		var_dump('sql-> '.$oProcFormulario->getRedatazo);
	}
	else{ //errores
		var_dump('mens retorno-> ', $oProcFormulario->getMensajes()); //problemas de validacion, devolver a la pagina con estos mensajes
		var_dump('error-> '.$oProcFormulario->getError()); //devolver a la pagina con mensaje de error (no necesariamente los de la propia clase)
	}
}
else{
	echo('Ningún formulario a procesar.');
}
 * \endcode
 *
 * @todo variable en la que se recoja la id que se ha insertado/actualizado/borrado y tabla
 * @author José M. Carnero
 * @version 4
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package formazo
 */
class Formazo_proc{

	/**
	 * Accion del formulario:
	 * ins - insertar
	 * upd- actualizar
	 * del -borrar
	 * busc -buscar
	 *
	 * @var string
	 * @see formazo_proc::setAccionForm($act),formazo_proc::getAccionForm()
	 * @access private
	 */
	private $sAccionForm;

	/**
	 * Cadena de texto a sumar al nombre de archivos (antes)
	 *
	 * @var string
	 * @see formazo_proc::setPrefijos($prefijo, $sufijo)
	 * @access private
	 */
	private $sArchPrefijo;

	/**
	 * Cadena de texto a sumar al nombre de archivos (despues, antes de la extension)
	 *
	 * @var string
	 * @see formazo_proc::setPrefijos($prefijo, $sufijo)
	 * @access private
	 */
	private $sArchSufijo;

	/**
	 * Campos del formulario
	 * Exactamente lo mismo que se use para generarlo/pintarlo, ver class.formazo.inc
	 *
	 * @var array
	 * @see formazo_proc::getCampos(),formazo_proc::setCampos($campos)
	 * @access private
	 */
	private $aCampos;

	/**
	 * Clave primaria para la fuente de datos (si son varias un array)
	 * y valores de la clave primaria para la fuente de datos
	 * Para actualizaciones y borrados
	 *
	 * @todo se contempla la posibilidad de inserciones sin claves autonumericas?
	 * @var string
	 * @see formazo_proc::setClavePrimaria($id, $val)
	 * @access private
	 */
	private $aClavePrimaria;

	/**
	 * Mensajes de error
	 *
	 * @var array
	 * @see formazo_proc::getError(),formazo_proc::setError()
	 * @access private
	 */
	private $aError;

	/**
	 * Mensajes de error la clase, a fin de que puedan ser traducidos/modificados
	 * Las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas, si es necesario poner un % se pondra como %%
	 *
	 * @var array
	 * @see formazo_proc::setErrorMensaje($clave, $valor),formazo_proc::getErrorMensaje($clave)
	 * @access private
	 */
	private $aErrorMensajes;

	/**
	 * Nombre del formulario a procesar (atributo "name" del formulario que envia los datos)
	 *
	 * @var string
	 * @see formazo_proc::setFormNombre($nombre),formazo_proc::getFormNombre()
	 * @access private
	 */
	private $sFormNombre;

	/**
	 * Tabla para la SQL, o fichero para CSV
	 *
	 * @var string
	 * @see formazo_proc::setFuenteDatos($fuente)
	 * @access private
	 */
	private $sFuenteDatos;

	/**
	 * Limpia el nombre de fichero; sustituye cualquier caracter no alfanumerico por el caracter asignado;
	 * si = false no hace limpiado
	 *
	 * @var string
	 * @see formazo_proc::setLimpiarNombreFichero($caracter)
	 * @access private
	 */
	private $sLimpiarNombreFichero;

	/**
	 * Mensaje de retorno para la pagina que llame a esta con los errores encontrados (datos no validos, no rellenados, etc)
	 *
	 * @var string
	 * @see formazo_proc::getMensajes(),formazo_proc::setMensajes()
	 * @access private
	 */
	private $aMensajes;

	/**
	 * Mensajes que devuelve la clase, a fin de que puedan ser traducidos/modificados;
	 * las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas,
	 * si es necesario poner un % se pondra como %%
	 *
	 * @var array
	 * @see formazo_proc::getMensajesRetorno($clave),formazo_proc::setMensajesRetorno($clave, $valor)
	 * @access private
	 */
	private $aMensajesRetorno;

	/**
	 * Controla si se renombran ficheros subidos en caso de que ya exista uno con el mismo nombre;
	 * true -> sobreescribe, false (defecto) -> renombra
	 *
	 * @var boolean
	 * @see formazo_proc::setPisarFicheros($pisar)
	 * @access private
	 */
	private $bPisarFicheros;

	/**
	 * Consulta para Redatazo construida a partir de los datos del formulario
	 *
	 * @var array
	 * @see formazo_proc::getRedatazo()
	 * @access private
	 */
	private $aRedatazo;

	/**
	 * Ruta (por omision) donde se subiran los ficheros; ej.: "/var/tmp/"
	 *
	 * @todo poner en cada campo de fichero una propiedad mas con este valor (y este por omision para todos)
	 * @var string
	 * @see formazo_proc::setRutaFicheros($ruta)
	 * @access private
	 */
	private $sRutaFicheros;

	/**
	 * Tiempos consumidos en la generacion del formulario, tanto total como de elementos, ...
	 *
	 * @var array
	 * @see formazo_proc::getTiempos()
	 * @access protected
	 */
	protected $aTiempos;

	/**
	 * En este array se recoge el de GET o POST, segun el que se haya utilizado para llegar a esta pagina; si no se queda con un array vacio
	 *
	 * @var array
	 * @see formazo_proc::setRequest(),formazo_proc::getVariablesRequest()
	 * @access private
	 */
	private $aVariablesRequest;

	/**
	 * Constructor
	 */
	function __construct($aCampos = array()){
		$this->aError = array();
		$this->setErrorMensaje(); //inicializacion de los mensajes de error
		$this->aTiempos = array('total' => 0); //medicion de tiempo

		$this->setRequest(); //recogida automatica de valores, por get o post

		$this->setFormNombre();
		//$this->setAccionForm();
		$this->setCampos($aCampos);
		$this->aRedatazo = array();
		$this->sFuenteDatos = '';
		$this->aClavePrimaria = array('', '');
		$this->sRutaFicheros = '';
		$this->sLimpiarNombreFichero = false; //si se desean nombres de ficheros limpios, asignar a esta propiedad una cadena vacia (''), o '_', o cualquier otro caracter que no sea problematico
		$this->bPisarFicheros = false;
		$this->aMensajes = array();
		$this->setMensajesRetorno();
		$this->sArchPrefijo = $this->sArchSufijo = '';
	}

	/**
	 * Devuelve el estado del objeto, error si lo hubiese, etc
	 * Informacion para debug
	 *
	 * @since 2011-09-15
	 * @access public
	 * @return string
	 */
	public function __toString(){
		$sRet = '';
		ob_start();

		if($this->getError()){
			echo('Errores: ');
			var_dump($this->getError());
		}

		if($this->getMensajes()){
			echo('Mensajes: ');
			var_dump($this->getMensajes());
		}

		echo('Redatazo: ');
		var_dump($this->getRedatazo());

		echo('Form method: '.$_SERVER['REQUEST_METHOD']."\n");
		echo('Form values: ');
		var_dump($this->aVariablesRequest);

		$sRet = ob_get_contents();
		ob_end_clean();

		return($sRet);
	}

	/**
	 * Devuelve la accion del formulario.
	 *
	 * @access public
	 * @return string
	 */
	public function getAccionForm(){
		return($this->sAccionForm);
	}

	/**
	 * Devuelve el estado actual de los campos del formulario
	 *
	 * @since 2011-09-27
	 * @access public
	 * @return array
	 */
	public function getCampos(){
		return($this->aCampos);
	}

	/**
	 * Devuelve los errores que se hayan producido
	 *
	 * @access public
	 * @return array
	 */
	public function getError(){
		return(empty($this->aError)?false:$this->aError);
	}

	/**
	 * Muestra el texto asignado a un error
	 * Si no se pasa ninguna clave se muestran todos
	 *
	 * @todo mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @access public
	 * @param $clave Clave del mensaje de error, si se omite (o false) muestra el array completo
	 * @return string
	 */
	public function getErrorMensaje($clave = false){
		if($clave === false) return($this->aErrorMensajes);
		return($this->aErrorMensajes[$clave]);
	}

	/**
	 * Devuelve el nombre del formulario
	 *
	 * @since 2011-09-14
	 * @access public
	 * @return string
	 */
	public function getFormNombre(){
		return($this->sFormNombre);
	}

	/**
	 * Devuelve mensajes de aviso, que pueden ser usados en la pagina de retorno del formulario para avisar de situaciones de error
	 *
	 * @since 2011-09-14
	 * @access public
	 * @return string
	 */
	public function getMensajes(){
		return(empty($this->aMensajes)?false:$this->aMensajes);
	}

	/**
	 * Muestra el texto asignado a un mensaje de retorno
	 * Si no se pasa ninguna clave se muestran todos
	 *
	 * @todo mostrar la clave de un texto de mensaje de retorno (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @access public
	 * @param $clave Clave del mensaje de retorno, si se omite (o false) muestra el array completo
	 * @return string
	 */
	public function getMensajesRetorno($clave = false){
		if($clave === false) return($this->aMensajesRetorno);
		return($this->aMensajesRetorno[$clave]);
	}

	/**
	 * Devuelve la consulta ya construida,en el formato de class.redatazo,
	 * en la forma:
	 * 'insert|update|delete' => array('fuente' => 'tabla|fichero', 'datos' => array('parametro1' => 'valor1', 'parametro2' => 'valor2'), 'where' => array('condicion' => 'valor'))
	 *
	 * @since 2015-03-22
	 * @access public
	 * @return array
	 */
	public function getRedatazo(){
		return($this->aRedatazo);
	}

	/**
	 * Checks date if matches given format and validity of the date.
	 *
	 * Examples:
	 * <code>
	 * is_date('22.22.2222', 'mm.dd.yyyy'); // returns false
	 * is_date('11/30/2008', 'mm/dd/yyyy'); // returns true
	 * is_date('30-01-2008', 'dd-mm-yyyy'); // returns true
	 * is_date('2008 01 30', 'yyyy mm dd'); // returns true
	 * </code>
	 *
	 * @todo cambiar el formato del dato si no es el que acepta la base de datos
	 * @access private
	 * @param string $value The variable being evaluated.
	 * @param string $format Format of the date. Any combination of mm, dd, yyyy with single character separator between.
	 * @return boolean
	 */
	private function is_date($value, $format = 'yyyy-mm-dd'){

		if(strlen($value) == 10 && strlen($format) == 10){

			//find separator. Remove all other characters from $format
			$separator_only = str_replace(array('m','d','y'),'', $format);
			$separator = $separator_only[0]; // separator is first character

			if($separator && strlen($separator_only) == 2){
				// make regex
				$regexp = str_replace('mm', '[0-1][0-9]', $value);
				$regexp = str_replace('dd', '[0-3][0-9]', $value);
				$regexp = str_replace('yyyy', '[0-9]{4}', $value);
				$regexp = str_replace($separator, "\\" . $separator, $value);

				if($regexp != $value && preg_match('/'.$regexp.'/', $value)){
					// check date
					$day   = substr($value,strpos($format, 'd'),2);
					$month = substr($value,strpos($format, 'm'),2);
					$year  = substr($value,strpos($format, 'y'),4);

					if(@checkdate($month, $day, $year)) return true;
				}
			}
		}
		return false;
	}

	/**
	 * Copyright 2005-2008 David Vidal Serra. Bajo licencia GNU GPL.
	 * Este software viene SIN NINGUN TIPO DE GARANTIA; para saber mas detalles
	 * puede consultar la licencia en http://www.gnu.org/licenses/gpl.txt(1)
	 * Esto es software libre, y puede ser usado y redistribuirdo de acuerdo
	 * con la condicion de que el autor jamas sera responsable de su uso.
	 * Returns: 1 = NIF ok, 2 = CIF ok, 3 = NIE ok, -1 = NIF bad, -2 = CIF bad, -3 = NIE bad, false = ??? bad
	 *
	 * @access private
	 * @param string $cif CIF/NIF/NIE
	 * @return integer
	 */
	private function is_nif($cif) {
		$cif = strtoupper($cif);

		for($i = 0; $i < 9; $i ++) $num[$i] = substr($cif, $i, 1);

		//si no tiene un formato valido devuelve error
		if(!ereg('((^[A-Z]{1}[0-9]{7}[A-Z0-9]{1}$|^[T]{1}[A-Z0-9]{8}$)|^[0-9]{8}[A-Z]{1}$)', $cif)) return 0;

		//comprobacion de NIFs estandar
		if(ereg('(^[0-9]{8}[A-Z]{1}$)', $cif))
			if($num[8] == substr('TRWAGMYFPDXBNJZSQVHLCKE', substr($cif, 0, 8) % 23, 1))
				return 1;
			else
				return -1;

		//algoritmo para comprobacion de codigos tipo CIF
		$suma = $num[2] + $num[4] + $num[6];
		for($i = 1; $i < 8; $i += 2) $suma += substr((2 * $num[$i]),0,1) + substr((2 * $num[$i]),1,1);
		$n = 10 - substr($suma, strlen($suma) - 1, 1);

		//comprobacion de NIFs especiales (se calculan como CIFs)
		if(ereg('^[KLM]{1}', $cif))
			if($num[8] == chr(64 + $n))
				return 1;
			else
				return -1;

		//comprobacion de CIFs
		if(ereg('^[ABCDEFGHJNPQRSUVW]{1}', $cif))
			if($num[8] == chr(64 + $n) || $num[8] == substr($n, strlen($n) - 1, 1))
				return 2;
			else
				return -2;

		//comprobacion de NIEs
		//T
		if(ereg('^[T]{1}', $cif))
			if($num[8] == ereg('^[T]{1}[A-Z0-9]{8}$', $cif))
				return 3;
			else
				return -3;

		//XYZ
		if(ereg('^[XYZ]{1}', $cif))
			if($num[8] == substr('TRWAGMYFPDXBNJZSQVHLCKE', substr(str_replace(array('X','Y','Z'), array('0','1','2'), $cif), 0, 8) % 23, 1))
				return 3;
			else
				return -3;

		//si todavia no se ha verificado devuelve error
		return 0;
	}

	/**
	 * Limpia nombres de ficheros
	 *
	 * Original: Cameron Perry (http://iamcam.wordpress.com/2007/03/20/clean-file-names-using-php-preg_replace/)
	 *
	 * @todo cortar el resultado (255 caracteres?) segun sistema operativo pueden dar problemas los nombres largos
	 * @todo opcion para convertir el nombre a camelCase
	 * @access private
	 * @param $nombreFichero Nombre del fichero a limpiar
	 * @return string
	 */
	private function limpiarNombreFichero($nombreFichero){
		if($this->sLimpiarNombreFichero === false) return($nombreFichero);

		$nombreFichero = html_entity_decode(strip_tags($nombreFichero));
		//$nombreFichero = ucwords($nombreFichero);
		$patron = "/([[:alnum:]_\.-]*)/";
		$nombreFichero = str_replace(str_split(preg_replace($patron, $this->sLimpiarNombreFichero, $nombreFichero)), $this->sLimpiarNombreFichero, $nombreFichero);
		//if(strlen($nombreFichero) > 255) $nombreFichero = substr($nombreFichero, 0, 255);

		return($nombreFichero);
	}

	/**
	 * Devuelve microtime en segundos
	 *
	 * @access private
	 * @param boolean $float Si true devuelve un numero flotante, si false devuelve formateado "legible para humanos" (parecido a ISO 8601)
	 * @return float
	 */
	private function microtimeSeg($float = false){
		list($mseg, $seg) = explode(" ", microtime());

		if($float) return((float)$mseg + (float)$seg);
		else return(date('Y-m-d H:i:s', $seg).substr($mseg, 1));
	}

	/**
	 * Procesar formulario
	 *
	 * @access public
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	public function procesarFormulario($nombreForm = ''){
		if($nombreForm != '') $this->setFormNombre($nombreForm);

		if($this->sFormNombre === false || $this->sFormNombre == ''){
			$this->setError(__METHOD__.'_1');
			return(false);
		}

		if(empty($this->aCampos)){
			$this->setError(__METHOD__.'_2', $this->sFormNombre);
			return(false);
		}

		if(!$this->verificaToken()){ //no coincide el token enviado con el generado, posible ataque
			$this->setError(__METHOD__.'_3');
			return(false);
		}


		if(!$this->procesarValores()){
			return(false); //errores en la recuperacion de valores del formulario
		}
		if(!$this->setRedatazo()){
			return(false); //errores en la construccion de la SQL
		}

		return(true);
	}

	/**
	 * Procesar los datos del formulario
	 *
	 * @access private
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	private function procesarValores(){
		//recoger valores
		foreach($this->aCampos as $clave => $valor){
			//directorio donde subir ficheros
			//if(isset($_SESSION[$this->sFormNombre]['frmRutaFicheros']) && $this->sRutaFicheros == '') $this->sRutaFicheros = $_SESSION[$this->sFormNombre]['frmRutaFicheros'];
			$this->sRutaFicheros = rtrim($this->sRutaFicheros, '\\/').'/'; //pone el ultimo /

			//acciones segun el tipo de elemento
			switch($valor['tipo']){
				case 'group'; //no se hace nada para estos, son elementos puramente decorativos
					/*if($valor['extra']['mode'] == 'repeat'){

					}
					break;*/
				case 'heading';
					break;
				case 'multiple';
					//$this->aCampos[$clave]['valor'] = implode(',', $sTempPost);
				case 'hidden':
				case 'password':
				case 'radio';
				case 'select';
				case 'text':
				case 'textarea':
					if(!$this->validarDato($clave, isset($valor['validacion'])?$valor['validacion']:'', isset($valor['obligatorio'])?$valor['obligatorio']:0)) return(false); //algun problema con la validacion
					break;
				case 'checkbox';
					if(isset($this->aVariablesRequest[$clave]) && strtolower($this->aVariablesRequest[$clave]) == 'on') $this->aCampos[$clave]['valor'] = 1;
					else $this->aCampos[$clave]['valor'] = 0;
					break;
				case 'file':
					$this->subeArchivo($clave); //TODO una mala subida de archivo no corta el proceso, debe ser asi?
					break;
				default:
					$this->setError(__METHOD__, $valor['tipo']);
					return(false);
			}
		}
		return(true);
	}

	/**
	 * Comprueba si existe el nombre del fichero pasado y construye un nombre que no exista
	 *
	 * @access private
	 * @param string $fichero Nombre del fichero
	 * @return string
	 */
	private function renombraArchivo($fichero){
		if(file_exists($this->sRutaFicheros.$fichero)){
			$aTempArchNombre = explode('.', $fichero);
			$ext = array_pop($aTempArchNombre); //extrae la extension
			$nombre = implode('.', $aTempArchNombre); //nombre del fichero, sin extension

			preg_match_all('/.*?(\d+)/i', $nombre, $nums, PREG_PATTERN_ORDER);
			if(count($nums[1])){ //se ha encontrado una cadena numerica, se usara la ultima encontrada para el siguiente sumatorio para hacer diferente el nombre de fichero
				$num = array_pop($nums[1]);
				$nums[0][count($nums[0])-1] = str_replace($num, '', $nums[0][count($nums[0])-1]);
				$nombre = implode('', $nums[0]); //nombre del fichero, sin extension y sin el numero final encontrado
				$num++;
			}
			else $num = '_0';

			return($this->renombraArchivo($nombre.$num.'.'.$ext));
		}

		return($fichero);
	}

	/**
	 * Asigna la acción del formulario
	 * Si no se le pasa nada asigna automaticamente con lo recibido
	 *
	 * @access public
	 * @param string $act Accion del formulario
	 * @return boolean
	 */
	public function setAccionForm($act = ''){
		//recuperada asi permite que haya mas de una accion en el formulario (ej. actualizar/&borrar)
		if(empty($act)) $act = isset($this->aVariablesRequest['frmAccion'])?$this->aVariablesRequest['frmAccion']:false;

		switch($act){
			case 'busc':
			case 'del':
			case 'ins':
			case 'upd':
				$this->sAccionForm = $act;
				break;
			default:
				$this->sAccionForm = false;
				$this->setError(__METHOD__, $act);
				return(false);
		}

		return(true);
	}

	/**
	 * Recoge y prepara los campos que formaran el formulario
	 *
	 * @since 2011-07-20
	 * @access public
	 * @param array $campos Array (o texto xml) con los campos que formaran el formulario
	 * @return boolean
	 */
	public function setCampos($campos = array()){
		//tipos que se ignoran en procesado (si esta a true)
		$aIgnorarTipos = array('checkbox' => false, 'file' => false, 'group' => true, 'heading' => true, 'hidden' => false, 'multiple' => false, 'password' => false, 'radio' => false, 'select' => false, 'text' => false, 'textarea' => false);

		if(!is_array($campos) && strpos(trim($campos), '<?xml') === 0){ //origen xml
			//medicion de tiempo
			$this->aTiempos['setCampos'] = $this->microtimeSeg(true);

			$this->aCampos = array();

			libxml_use_internal_errors(false);
			$xmlArray = simplexml_load_string($campos, 'SimpleXMLElement', LIBXML_NOCDATA); //necesario para leer secciones CDATA
			if($xmlArray === false){
				$this->setError(__METHOD__, implode(' ## ', libxml_get_errors()));
				libxml_clear_errors();
				return(false);
			}

			foreach($xmlArray->attributes() as $clave => $valor){ //guardar id del formulario
				if($clave == 'id') $this->sFormNombre = (string)$valor;
			}
			foreach($xmlArray->children() as $valor){ //convertir elementos de xml en elementos del formulario
				$this->xmlForm($valor);
			}

			foreach($this->aCampos as $clave => $valor){
				if(!isset($valor['tipo']) || !empty($aIgnorarTipos[$valor['tipo']])) unset($this->aCampos[$clave]); //evita procesar campos dedicados exclusivamente al pintado de listados y aquellos que son solo decorativos
			}

			//medicion de tiempo
			$this->aTiempos['setCampos'] = $this->microtimeSeg(true) - $this->aTiempos['setCampos'];
		}
		elseif(count($campos)){ //origen array
			$this->aCampos = array();
			if(!is_array($campos)){
				$this->setError(__METHOD__.'_2');
				return(false);
			}

			foreach($campos as $clave => $valor){
				if(isset($valor['tipo']) &&  empty($aIgnorarTipos[$valor['tipo']])) $this->aCampos[$clave] = $valor; //evita procesar campos dedicados exclusivamente al pintado de listados y aquellos que son solo decorativos
			}
		}
		else{ //vacio, origen desconocido o ya asignado en formazo::setCampos()
			if(empty($this->aCampos)){
				$this->setError(__METHOD__.'_3');
				return(false);
			}
		}

		return(true);
	}

	/**
	 * Guarda los errores que se hayan producido
	 *
	 * @access private
	 * @return string
	 */
	private function setError(){
		$argumentos = func_get_args();
		if(empty($argumentos)) return(false);

		$clave = array_shift($argumentos);

		$this->aError[] = '['.$this->microtimeSeg().'] '.vsprintf($this->aErrorMensajes[$clave], $argumentos);
		//trigger_error($this->aError[count($this->aError)-1]);

		return(true);
	}

	/**
	 * Cambia mensajes de error (su texto)
	 * Util, por ejemplo, para traducciones
	 * Si clave igual a false se reinicializan los mensajes
	 *
	 * @access public
	 * @param mixed $clave Clave del mensaje de error, o un array con todos los mensajes de error (array('clave' => 'mensaje de error',[...]);)
	 * @param string $valor Nuevo texto para el mensaje de error
	 * @return boolean
	 */
	public function setErrorMensaje($clave = false, $valor = false){
		if($clave === false){ //reinicio de mensajes
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aErrorMensajes = array(
			'Formazo_proc::construirSQL_1' => 'No se ha pasado acci&oacute;n para el formulario.',
			'Formazo_proc::construirSQL_2' => 'El tipo de acci&oacute;n (-%s-) no est&aacute; admitido.',
			'Formazo_proc::construirSQL_3' => 'Tipo de elemento desconocido -%s-.',
			'Formazo_proc::procesarFormulario_1' => 'Nombre de formulario vac&iacute;o.',
			'Formazo_proc::procesarFormulario_2' => 'No hay datos para procesar el formulario <em>%s</em>.',
			'Formazo_proc::procesarValores' => 'El tipo de campo <em>%s</em> no se reconoce.',
			'Formazo_proc::setAccionForm' => 'El tipo de acci&oacute;n (-%s-) no est&aacute; admitido.',
			'Formazo_proc::setCampos' => 'Error XML: %s',
			'Formazo_proc::setCampos_2' => 'No es una matriz de campos válida.',
			'Formazo_proc::setCampos_3' => 'No se puede crear un formulario sin campos.',
			'Formazo_proc::setRutaFicheros' => 'No dispone de permisos para subir ficheros al directorio <em>%s</em>',
			'Formazo_proc::subeArchivo_1' => 'No está permitido el tipo de fichero <em>%s</em>.',
			'Formazo_proc::subeArchivo_2' => 'No se ha podido borrar el archivo <em>%s</em> en la ruta <em>%s</em>.',
			'Formazo_proc::subeArchivo_3' => 'No se ha podido subir el archivo <em>%s</em> a la ruta <em>%s</em>.'
			);
			return(true);
		}

		$aMensajes = array();

		if(!is_array($clave)) $aMensajes[$clave] = $valor;
		else $aMensajes = $clave;

		foreach($aMensajes as $c => $v){
			$this->aErrorMensajes[$c] = $v;
		}
		return(true);
	}

	/**
	 * Recoge y prepara los campos que formaran el formulario
	 *
	 * @since 2011-09-14
	 * @access public
	 * @param string $nombre Nombre del formulario
	 * @return boolean
	 */
	public function setFormNombre($nombre = false){
		$this->sFormNombre = empty($nombre)?(isset($this->aVariablesRequest['frmFormName']) && $this->aVariablesRequest['frmFormName'] != ''?$this->aVariablesRequest['frmFormName']:false):$nombre;

		return(true);
	}

	/**
	 * Asigna el caracter con el que se limpiaran los nombres de fichero,
	 * el caracter asignado (incluido nulo: '') sustituira a todos los caracteres no alfanumericos,
	 * con false no se hace limpiado, solo toma el primer caracter de la cadena pasada
	 *
	 * @since 2011-09-14
	 * @access public
	 * @param string $caracter Caracter para limpiar
	 * @return boolean
	 */
	public function setLimpiarNombreFichero($caracter = false){
		$this->sLimpiarNombreFichero = strlen($caracter) > 1?substr($caracter, 0, 1):$caracter;

		return(true);
	}

	/**
	 * Guarda los mensajes de retorno
	 *
	 * @access private
	 * @return string
	 */
	private function setMensajes(){
		$argumentos = func_get_args();
		if(empty($argumentos)) return(false);

		$clave = array_shift($argumentos);

		$this->aMensajes[] = vsprintf($this->aMensajesRetorno[$clave], $argumentos);

		return(true);
	}

	/**
	 * Cambia mensajes de retorno (su texto)
	 * Util, por ejemplo, para traducciones
	 * Si clave igual a false se reinicializan los mensajes
	 *
	 * @access public
	 * @param mixed $clave Clave del mensaje de retorno, o un array con todos los mensajes de retorno (array('clave' => 'mensaje de retorno',[...]);)
	 * @param string $valor Nuevo texto para el mensaje de retorno
	 * @return boolean
	 */
	public function setMensajesRetorno($clave = false, $valor = false){
		if($clave === false){ //reinicio de mensajes
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aMensajesRetorno = array(
			'Formazo_proc::validarDato' => '%s',
			'Formazo_proc::validarDato_2' => 'El campo <em>%s</em> tiene caracter obligatorio.',
			'Formazo_proc::validarDato_3' => 'El campo </em>%s</em> no es una dirección de email correcta y debe serlo.',
			'Formazo_proc::validarDato_4' => 'El campo <em>%s</em> no es entero (númerico) y debe serlo.',
			'Formazo_proc::validarDato_5' => 'El campo </em>%s</em> no es una fecha correcta con el formato </em>%s</em> y debe serlo.',
			'Formazo_proc::validarDato_6' => 'El campo </em>%s</em> no es un documento válido y debe serlo.',
			'Formazo_proc::validarDato_7' => 'El campo <em>%s</em> no es numérico y debe serlo.',
			'Formazo_proc::validarDato_8' => 'El campo </em>%s</em> no es una URL correcta y debe serlo.'
			);
			return(true);
		}

		$aMensajes = array();

		if(!is_array($clave)) $aMensajes[$clave] = $valor;
		else $aMensajes = $clave;

		foreach($aMensajes as $c => $v){
			$this->aMensajesRetorno[$c] = $v;
		}
		return true;
	}

	/**
	 * Indica si los ficheros subidos pisaran (true) a los que ya existan con el mismo nombre o no (false)
	 *
	 * @since 2011-09-14
	 * @access public
	 * @param boolean $pisar Pisar (true) ficheros o no (false)
	 * @return boolean
	 */
	public function setPisarFicheros($pisar = false){
		$this->bPisarFicheros = $pisar;

		return true;
	}

	/**
	 * Asigna prefijo y sufijo que se añadiran a los nombres de archivo
	 *
	 * @access public
	 * @param string $prefijo Prefijo
	 * @param string $sufijo Sufijo
	 * @return boolean
	 */
	public function setPrefijos($prefijo = '', $sufijo = ''){
		$this->sArchPrefijo = $prefijo;
		$this->sArchSufijo = $sufijo;

		return(true);
	}

	/**
	 * Construye la consulta para Redatazo
	 *
	 * @access private
	 * @return boolean
	 */
	private function setRedatazo(){
		if(empty($this->sAccionForm)){
			$this->setError(__METHOD__.'_1');
			return false;
		}

		$sTipo = '';

		//tipo de query
		switch($this->sAccionForm){
			case 'ins':
				$sTipo = 'insert';
				break;
			case 'upd':
				$sTipo = 'update';
				break;
			case 'del':
				$sTipo = 'delete';
				break;
			case 'busc':
				$sTipo = 'busqueda';
				break;
			default:
				$this->setError(__METHOD__.'_2', $this->sAccionForm);
				return false;
		}

		$aDatos = array();
		//datos query
		if($this->sAccionForm != 'del'){ //no hay validaciones en borrado
			foreach($this->aCampos as $clave => $valor){
				switch($this->sAccionForm){
					case 'ins':
					case 'upd':
						if($valor['tipo'] == 'file'){
							if(!empty($valor['valor'])) $aDatos[$clave] = $valor['valor'];
							elseif(!empty($this->aVariablesRequest['hid_'.$clave])) $aDatos[$clave] = $this->aVariablesRequest['hid_'.$clave];
							else $aDatos[$clave] = '';
						}
						elseif($valor['tipo'] == 'multiple'){
							if(!isset($valor['separador'])) $valor['separador'] = ',';
							$aDatos[$clave] = implode($valor['separador'], $valor['valor']);
						}
						else{
							if(empty($valor['obligatorio']) && ($valor['validacion'] == 'numerico' || $valor['validacion'] == 'entero') && (empty($valor['valor']) && $valor['valor'] != 0)){
								$aDatos[$clave] = NULL; //si es numerico y no obligatorio -> NULL
							}
							else $aDatos[$clave] = isset($valor['valor'])?$valor['valor']:NULL;
						}
						break;
					case 'del':
					case 'busc':
						break;
					default:
						$this->setError(__METHOD__.'_3', $valor['tipo']);
						//return(false);
				}
			}
		}

		$aWhere = array();
		//condiciones query
		switch($this->sAccionForm){
			case 'upd':
				if(is_array($this->aClavePrimaria[0])){
					for($i=0;$i<count($this->aClavePrimaria[0]);$i++){
						$aWhere[$this->aClavePrimaria[0][$i]] = $this->aClavePrimaria[1][$i];
					}
				}
				else{
					$aWhere[$this->aClavePrimaria[0]] = $this->aClavePrimaria[1];
				}
				break;
			case 'del':
				if(is_array($this->aClavePrimaria[0])){
					for($i=0;$i<count($this->aClavePrimaria[0]);$i++){
						$aWhere[$this->aClavePrimaria[0][$i]] = $this->aClavePrimaria[1][$i];
					}
				}
				else{
					$aWhere[$this->aClavePrimaria[0]] = $this->aClavePrimaria[1];
				}
				break;
			case 'ins':
			case 'busc':
			default:
		}

		$this->aRedatazo[$sTipo] = array(
	 		'fuente' => $this->sFuenteDatos,
			'datos' => $aDatos,
			'where' => $aWhere
		);

		return true;
	}

	/**
	 * Procesar los datos del formulario
	 *
	 * @access private
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	private function setRequest(){
		//recoge GET o POST segun el que se haya utilizado para llegar a esta pagina
		if($_SERVER['REQUEST_METHOD'] == 'GET') $this->aVariablesRequest = &$_GET;
		elseif($_SERVER['REQUEST_METHOD'] == 'POST') $this->aVariablesRequest = &$_POST;
		else $this->aVariablesRequest = array(); //TODO recoger $_REQUEST?

		$this->setAccionForm();
		//cambia la accion del formulario en aquellos con doble accion
		//if(isset($this->sAccionForm) && $this->sAccionForm == 'upd' && !empty($_SESSION[$this->aVariablesRequest['frmFormName']]['frmAccionUpdDel'])) $this->sAccionForm = $this->aVariablesRequest['frmAccion'];
		return true;
	}

	/**
	 * Asigna el nombre de la clave primaria de la fuente de datos
	 * Y asigna el valor de la clave primaria de la fuente de datos
	 *
	 * @since 2011-09-15
	 * @access public
	 * @param string $id Nombre del id de la tabla (puede ser un array de ellos)
	 * @param string $id Valor del id de la tabla (puede ser un array de ellos)
	 * @return boolean
	 */
	public function setClavePrimaria($id = '', $val = ''){
		$this->aClavePrimaria[0] = $id;
		$this->aClavePrimaria[1] = $val;

		return true;
	}

	/**
	 * Asigna la ruta (por omision) donde se guardaran los ficheros que se suban,
	 * solo se usa en ficheros que no llevan su propia ruta de guardado
	 *
	 * @since 2011-09-14
	 * @access public
	 * @param string $ruta Ruta de guardado
	 * @return boolean
	 */
	public function setRutaFicheros($ruta = false){
		if($ruta === false || $ruta == '') return(false); //permite la ruta "0"
		if(!is_writable($ruta)){
			$this->setError(__METHOD__, $ruta);
			return(false);
		}
		$this->sRutaFicheros = rtrim($ruta, '\\/').'/'; //pone el ultimo /

		return true;
	}

	/**
	 * Asigna la fuente de datos para las consulas (tabla SQL, fichero CSV, ...)
	 *
	 * @since 2011-09-15
	 * @access public
	 * @param string $fuente Fuente de datos
	 * @return boolean
	 */
	public function setFuenteDatos($fuente = ''){
		$this->sFuenteDatos = $fuente;

		return true;
	}

	/**
	 * Sube ficheros.
	 *
	 * @todo posibilidad de subir varios archivos del mismo formulario a distintos lugares
	 * @todo validacion de ficheros (ya puesta en generador) por extension permitida
	 * @access private
	 * @param string $campo Clave del campo que contiene el fichero a subir
	 * @return boolean
	 */
	private function subeArchivo($campo){
		if(isset($_FILES[$campo]) && is_uploaded_file($_FILES[$campo]['tmp_name'])){
			$sTempArchNombre = '';

			//validando tipo de fichero
			if(isset($_FILES[$campo]) && !empty($this->aCampos[$campo]['validacion']) && $this->aCampos[$campo]['validacion'] == 'fichero' && isset($this->aCampos[$campo]['extra']['modo'])){
				$tipos = ','.$this->aCampos[$campo]['extra']['modo'].',';
				$fileExt = strrpos($_FILES[$campo]['name'], '.');
				if($fileExt === false) $fileExt = '';
				else $fileExt = substr($_FILES[$campo]['name'], $fileExt + 1);

				if(strpos($tipos, ','.$fileExt.',') === false){
					$this->setError(__METHOD__.'_1', $_FILES[$campo]['name']);
					return(false);
				}
			}

			//si se ha subido algun fichero y es un "UPDATE" se borran los ficheros previos
			if($this->aVariablesRequest['frmAccion'] == 'upd' && isset($this->aVariablesRequest['hid_'.$campo]) && $this->aVariablesRequest['hid_'.$campo] != '' && file_exists($this->sRutaFicheros.$this->aVariablesRequest['hid_'.$campo])){
				if(!unlink($this->sRutaFicheros.$this->aVariablesRequest['hid_'.$campo])){ //borra fichero
					$this->setError(__METHOD__.'_2', $_FILES[$campo]['name'], $this->sRutaFicheros);
					return(false);
				}
			}

			//poner prefijo y sufijo
			$sTempArchNombre = $this->sArchPrefijo.$_FILES[$campo]['name'];
			$aTempArchNombre = array();
			if(!empty($this->sArchSufijo)){
				$aTempArchNombre = explode('.', $sTempArchNombre);
				$sTempArchNombre = array_pop($aTempArchNombre); //extrae extension
				$sTempArchNombre = implode('.', $aTempArchNombre).$this->sArchSufijo.'.'.$sTempArchNombre;
			}

			if($this->sLimpiarNombreFichero !== false) $sTempArchNombre = $this->limpiarNombreFichero($sTempArchNombre);
			if(!$this->bPisarFicheros) $sTempArchNombre = $this->renombraArchivo($sTempArchNombre);

			//mover el archivo a su ubicacion definitiva
			if(!(@move_uploaded_file($_FILES[$campo]['tmp_name'], $this->sRutaFicheros.$sTempArchNombre))){
				$this->setError(__METHOD__.'_3', $sTempArchNombre, $this->sRutaFicheros);
				return(false);
			}
			else{ //guardado del nombre del fichero subido
				$this->aCampos[$campo]['valor'] = rawurlencode($sTempArchNombre);
			}
		}
		//borrado de ficheros cuando se borra el registro
		elseif($this->aVariablesRequest['frmAccion'] == 'del' && isset($this->aVariablesRequest['hid_'.$campo]) && $this->aVariablesRequest['hid_'.$campo] != '' && file_exists($this->sRutaFicheros.$this->aVariablesRequest['hid_'.$campo])){
			if(!unlink($this->sRutaFicheros.$this->aVariablesRequest['hid_'.$campo])){
				$this->setError(__METHOD__.'_2', $_FILES[$campo]['name'], $this->sRutaFicheros);
				return(false);
			}
		}
		//borrado con el boton de cada archivo en el formulario
		//TODO asegurar el borrado de ficheros, mandandolo por post es posible borrar cualquiera
		elseif(isset($this->aVariablesRequest['hid_'.$campo.'d']) && $this->aVariablesRequest['hid_'.$campo.'d'] == 'del'){
			if(! @unlink($this->sRutaFicheros.$this->aVariablesRequest['hid_'.$campo])){
				$this->setError(__METHOD__.'_2', $this->aVariablesRequest['hid_'.$campo], $this->sRutaFicheros);
				return(false);
			}
			$this->aCampos[$campo]['valor'] = $this->aVariablesRequest['hid_'.$campo] = '';
		}
		/*else
			return(false); //no necesariamente situacion de error: se ha pedido subir un archivo de un campo sin archivo
		}*/

		return(true);
	}

	/**
	 * Recoge y valida los datos del formulario
	 *
	 * @todo validaciones de "multiple"
	 * @access private
	 * @param string $campo Nombre de campo del formulario
	 * @param string $validacion Validación
	 * @param string $obligatorio Obligatorio
	 * @return boolean
	 */
	private function validarDato($campo, $validacion = '-', $obligatorio = 0){
			//verificar validaciones y recoger valores
			//$sTempPost = 'NULL'; //se pondra a nulo si no tiene valor correcto

			if($this->sAccionForm == 'del') return(true); //sin validaciones en borrado

			//obligatorio
			if(is_string($obligatorio) && function_exists($obligatorio)){
				$sTemp = eval($obligatorio.'()');
				if($sTemp !== true){
					$this->setMensajes(__METHOD__, $sTemp);
					return(false);
				}
			}
			elseif($obligatorio == 1){
				if(!isset($this->aVariablesRequest[$campo]) || $this->aVariablesRequest[$campo] == ''){
					$this->setMensajes(__METHOD__.'_2', $campo);
					return(false);
				}
			}

			//TODO longitud maxima

			//validaciones
			if(!empty($this->aVariablesRequest[$campo])){ //obligatorio ya se controla mas arriba
				$sTempPost = $this->aVariablesRequest[$campo];

				switch($validacion){
					case 'clave':
						break;
					case 'email':
						if(!preg_match('/\A([-a-zA-Z0-9_.])+\@(([-a-zA-Z0-9])+\.)+([a-zA-Z0-9]{2,4})+\Z/i', $sTempPost)) $this->setMensajes(__METHOD__.'_3', $campo);
						//else $sTempPost = "'".$sTempPost."'";
						break;
					case 'entero':
						if(!is_numeric($sTempPost)) $this->setMensajes(__METHOD__.'_4', $campo);
						break;
					case 'fecha':
						$formato = 'yyyy-mm-dd';
						if(!$this->is_date($sTempPost, $formato)) $this->setMensajes(__METHOD__.'_5', $campo, $formato);
						//else $sTempPost = "'".$sTempPost."'";
						break;
					case 'nif':
						if($this->is_nif($sTempPost) <= 0){
							$this->setMensajes(__METHOD__.'_6', $campo);
							return(false);
						}
						//else $sTempPost = "'".$sTempPost."'";
						break;
					case 'numerico':
						if(!is_numeric($sTempPost)) $this->setMensajes(__METHOD__.'_7', $campo);
						break;
					case 'telefono':
						break;
					case 'url':
/*busca urls fqdn (http://www.dominio.com/pag -> si, www.dominio.com/pag -> no)
((?#
    the scheme:
  )(?:https?://)(?#
    second level domains and beyond:
  )(?:[\S]+\.)+((?#
    top level domains:
  )MUSEUM|TRAVEL|AERO|ARPA|ASIA|EDU|GOV|MIL|MOBI|(?#
  )COOP|INFO|NAME|BIZ|CAT|COM|INT|JOBS|NET|ORG|PRO|TEL|(?#
  )A[CDEFGILMNOQRSTUWXZ]|B[ABDEFGHIJLMNORSTVWYZ]|(?#
  )C[ACDFGHIKLMNORUVXYZ]|D[EJKMOZ]|(?#
  )E[CEGHRSTU]|F[IJKMOR]|G[ABDEFGHILMNPQRSTUWY]|(?#
  )H[KMNRTU]|I[DELMNOQRST]|J[EMOP]|(?#
  )K[EGHIMNPRWYZ]|L[ABCIKRSTUVY]|M[ACDEFGHKLMNOPQRSTUVWXYZ]|(?#
  )N[ACEFGILOPRUZ]|OM|P[AEFGHKLMNRSTWY]|QA|R[EOSUW]|(?#
  )S[ABCDEGHIJKLMNORTUVYZ]|T[CDFGHJKLMNOPRTVWZ]|(?#
  )U[AGKMSYZ]|V[ACEGINU]|W[FS]|Y[ETU]|Z[AMW])(?#
    the path, can be there or not:
  )(/[a-z0-9\._/~%\-\+&\#\?!=\(\)@]*)?)*/

	//if(!preg_match('~\A(?:((https?|ftp)\:\/\/)?([a-z0-9+!*(),;?&=$_.-]+(\:[a-z0-9+!*(),;?&=$_.-]+)?@)?([a-z0-9-.]*)\.([a-z]{2,3})(\:[0-9]{2,5})?(\/([a-z0-9+$_-]\.?)+)*\/?(\?[a-z+&$_.-][a-z0-9;:@&%=+/$_.-]*)?(#[a-z_.-][a-z0-9+$_.-]*)?)\Z~i', $sTempPost)) $this->setMensajes('formularioProc::validarDato8', $campo);
	if(!preg_match('~\A(?:(?#
protocolo)(https?\:\/\/|ftps?\:\/\/)?(?#
dominio)([\w\.\-]{1,}\.\w{2,6})?(?#
puerto)(\:\d{1,5})?(?#
directorios)(\/[\w;@%=+$\.\-]{1,})*(?#
fichero)(\/[\w+$\.\-]*\.\w*)?(?#
get)(\?([a-z0-9+$_\.\-]&?)*)?(?#
ancla)(#[a-z_\.\-][a-z0-9+$_\.\-]*)?)\Z~i', $sTempPost)) $this->setMensajes('formularioProc::validarDato8', $campo);
						//else $sTempPost = "'".$sTempPost."'";
						break;
					default:
						//no se hace validacion
				}

				$this->aCampos[$campo]['valor'] = $sTempPost;
			}

			return(true);
	}

	/**
	 * Verifica el token de seguridad enviado con el formulario
	 *
	 * @todo verificar funcionamiento
	 * @todo comprobar si ip, sesion y user_agent se establecen correctamente cuando se envia por ajax o se redirecciona el procesado
	 * @since 2012-09-14
	 * @access private
	 * @param boolean $session Controla datos relacionados con sesion (true) o no
	 * @return boolean
	 */
	private function verificaToken($session = true){
		//formulario y procesado deben estar en la misma maquina
		$sSesion = $sUserAgent = '';
		if($session){
			@session_start(); //en PHP 4.3.3 o superior puede dar un E_NOTICE si se llama por segunda vez
			$sSesion = session_id();
			$sUserAgent = empty($_SERVER['HTTP_USER_AGENT'])?'':$_SERVER['HTTP_USER_AGENT'];
		}
		$sServer = empty($_SERVER['SERVER_ADDR'])?'':$_SERVER['SERVER_ADDR'];
		if(!empty($_SERVER['SERVER_PORT'])) $sServer .= $_SERVER['SERVER_PORT'];
		if(!empty($_SERVER['SERVER_SOFTWARE'])) $sServer .= $_SERVER['SERVER_SOFTWARE'];
		if(!empty($_SERVER['SERVER_NAME'])) $sServer .= $_SERVER['SERVER_NAME'];
		$sIp = empty($_SERVER['HTTP_X_FORWARDED_FOR'])?(empty($_SERVER['REMOTE_ADDR'])?'999:999:999:999':$_SERVER['REMOTE_ADDR']):$_SERVER['HTTP_X_FORWARDED_FOR'];
		$aCampos = /*empty($this->aCampos)?*/array()/*:array_keys($this->aCampos)*/;
		$token = $this->sFormNombre.'/'.$sSesion.'/'.implode('', $aCampos).'/'.$sUserAgent.'/'.$sIp.'/'.$sServer;

		return(!(empty($this->aVariablesRequest['frmTkn']) || md5($token) !== $this->aVariablesRequest['frmTkn']));
	}

	/**
	 * Convierte un documento XML con los elementos del formulario en el array normalizado
	 *
	 * @since 2011-07-07
	 * @access private
	 * @param object $xml Objeto SimpleXML
	 * @return boolean
	 */
	private function xmlForm($xml, $group = false){
		//tipo de elemento, corresponde con el nombre de cada objeto o subobjeto xml
		$tipo = $xml->getName();

		//id y atributos
		foreach($xml->attributes() as $clave => $valor) $$clave = (string)$valor;

		if(strtolower($tipo) == 'repeat'){
			$tipo = 'group';
			$mode = 'repeat';
		}
		$this->aCampos[$id]['tipo'] = $tipo; //TODO revisar grupos (por ejemplo para group/block/tab)
		if(!empty($validacion)) $this->aCampos[$id]['validacion'] = $validacion;
		if(!empty($required)) $this->aCampos[$id]['obligatorio'] = $required;
		if(!empty($longitud)) $this->aCampos[$id]['longitud'] = $longitud;
		if(!empty($depende)) $this->aCampos[$id]['depende']['id'] = $depende;
		if(!empty($depende_val)) $this->aCampos[$id]['depende']['val'] = $depende_val;
		if(!empty($ligado)) $this->aCampos[$id]['ligado'] = $ligado;
		if(!empty($separador)) $this->aCampos[$id]['separador'] = $separador;
		if(!empty($css)) $this->aCampos[$id]['css'] = $css;
		if(!empty($mode)) $this->aCampos[$id]['extra']['modo'] = $mode;

		if(!empty($group)) $this->aCampos[$id]['group'] = $group;

		//resto de propiedades y otros elementos contenidos
		foreach($xml->children() as $clave => $valor){
			switch($clave){
				case 'label':
					$this->aCampos[$id]['titulo'] = (string)$valor;
					break;
				case 'value':
					$this->aCampos[$id]['valor'] = (string)$valor;
					break;
				case 'choices':
					$tempChoices = $valor->children();
					foreach($tempChoices->item as $claveCh => $valorCh){
						$this->aCampos[$id]['valores'][(string)$tempChoices->label][(string)$valorCh->value] = (string)$valorCh->label;
					}
					break;
				case 'item':
					$this->aCampos[$id]['valores'][(string)$valor->value] = (string)$valor->label;
					break;
				case 'help';
				case 'hint';
					$this->aCampos[$id][$clave] = (string)$valor;
					break;
				default:
					$this->xmlForm($valor, ($tipo=='group'?$id:false));
					//if($valor->getName());
			}
		}

		//echo $xmlArray->movie[0]->plot;
		//echo $xml->movie->{'great-lines'}->line;
		return(true);
	}

}
