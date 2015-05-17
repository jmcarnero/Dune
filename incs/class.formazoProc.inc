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
	include_once("inc/class.formazoProc.inc");

	$oProcFormulario = new FormazoProc();

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
class FormazoProc {

	/**
	 * Campos del formulario
	 * Exactamente lo mismo que se use para generarlo/pintarlo, ver class.formazo.inc
	 *
	 * @var array
	 * @see FormazoProc::setCampos($campos)
	 */
	private $aCampos;

	/**
	 * Mensajes de error
	 *
	 * @var array
	 * @see FormazoProc::getError(),FormazoProc::setError()
	 */
	private $aError;

	/**
	 * Mensajes de error la clase, a fin de que puedan ser traducidos/modificados
	 * Las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas, si es necesario poner un % se pondra como %%
	 *
	 * @var array
	 * @see FormazoProc::setErrorMensaje($clave, $valor)
	 */
	private $aErrorMensajes;

	/**
	 * Nombre del formulario a procesar (atributo "name" del formulario que envia los datos)
	 *
	 * @var string
	 * @see FormazoProc::setFormNombre($nombre)
	 */
	private $sFormNombre;

	/**
	 * En este array se recoge el de GET o POST, segun el que se haya utilizado para llegar a esta pagina; si no se queda con un array vacio
	 *
	 * @var array
	 * @see FormazoProc::getValor($campo),FormazoProc::setRequest()
	 */
	private $aVariablesRequest;

	/**
	 * Constructor
	 */
	function __construct($aCampos = array()){
		$this->aError = array();
		$this->setErrorMensaje(); //inicializacion de los mensajes de error

		$this->setRequest(); //recogida automatica de valores, por get o post

		$this->setFormNombre();
		$this->setCampos($aCampos);
	}

	/**
	 * Devuelve los errores que se hayan producido
	 *
	 * @return array
	 */
	public function getError(){
		return empty($this->aError) ? false : $this->aError;
	}

	/**
	 * Procesar formulario
	 *
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	public function procesarFormulario($nombreForm = ''){
		if($nombreForm != '') $this->setFormNombre($nombreForm);

		if($this->sFormNombre === false || $this->sFormNombre == ''){
			trigger_error('Nombre de formulario vac&iacute;o', E_USER_ERROR);
			return false;
		}

		if(empty($this->aCampos)){
			trigger_error('No hay datos para procesar el formulario <em>$this->sFormNombre</em>', E_USER_WARNING);
			return false;
		}

		if(!$this->verificaToken()){ //no coincide el token enviado con el generado, posible ataque
			trigger_error('El token generado y el recibido no coinciden, posible ataque', E_USER_WARNING);
			return false;
		}


		if(!$this->procesarValores()){
			return false; //errores en la recuperacion de valores del formulario
		}
		if(!$this->setRedatazo()){
			return false; //errores en la construccion de la SQL
		}

		return true;
	}

	/**
	 * Procesar los datos del formulario
	 *
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	private function procesarValores(){
		$bRet = true;

		//recoger valores
		foreach($this->aCampos as $clave => $valor){
			//acciones segun el tipo de elemento
			switch($valor['tipo']){
				case 'multiple';
				case 'hidden':
				case 'password':
				case 'radio';
				case 'select';
				case 'text':
				case 'textarea':
					if(!$this->valida($clave, isset($valor['validacion']) ? $valor['validacion'] : '', isset($valor['obligatorio']) ? $valor['obligatorio'] : 0))
						$bRet = false; //algun problema con la validacion
					break;
				case 'checkbox';
					if(isset($this->aVariablesRequest[$clave]) && strtolower($this->aVariablesRequest[$clave]) == 'on')
						$this->aCampos[$clave]['valor'] = 1;
					else
						$this->aCampos[$clave]['valor'] = 0;
					break;
				case 'file':
					$this->subeArchivo($clave); //TODO una mala subida de archivo no corta el proceso, debe ser asi?
					break;
				default:
					trigger_error('El tipo de campo [<em>' . $valor['tipo'] . '</em>] no se reconoce', E_USER_NOTICE);
					$bRet = false;
			}
		}

		return $bRet;
	}

	/**
	 * Recoge y prepara los campos que formaran el formulario
	 *
	 * @since 2011-07-20
	 * @param array $campos Array (o texto xml) con los campos que formaran el formulario
	 * @return boolean
	 */
	public function setCampos($campos = array()){
		//tipos que se ignoran en procesado (si esta a true)
		$aIgnorarTipos = array('checkbox' => false, 'file' => false, 'group' => true, 'heading' => true, 'hidden' => false, 'multiple' => false, 'password' => false, 'radio' => false, 'select' => false, 'text' => false, 'textarea' => false);

		if(count($campos)){ //origen array
			$this->aCampos = array();
			if(!is_array($campos)){
				trigger_error('Matriz de campos NO válida', E_USER_WARNING);
				return false;
			}

			foreach($campos as $clave => $valor){
				if(isset($valor['tipo']) &&  empty($aIgnorarTipos[$valor['tipo']])) $this->aCampos[$clave] = $valor; //evita procesar campos dedicados exclusivamente al pintado de listados y aquellos que son solo decorativos
			}
		}
		else{ //vacio, origen desconocido o ya asignado en formazo::setCampos()
			if(empty($this->aCampos)){
				trigger_error('No se puede crear un formulario sin campos', E_USER_ERROR);
				return false;
			}
		}

		return true;
	}

	/**
	 * Guarda los errores a mostrar al usuario (como validaciones) que se hayan producido
	 *
	 * @return boolean
	 */
	private function setError(){
		$argumentos = func_get_args();
		if(empty($argumentos)) return false;

		$clave = array_shift($argumentos);

		$this->aError[] = vsprintf($this->aErrorMensajes[$clave], $argumentos);
		//trigger_error($this->aError[count($this->aError)-1]);

		return true;
	}

	/**
	 * Cambia mensajes de error (su texto)
	 * Util, por ejemplo, para traducciones
	 * Si clave igual a false se reinicializan los mensajes
	 *
	 * @todo se pierden los mensajes iniciales de las clases herederas
	 * @param mixed $clave Clave del mensaje de error, o un array con todos los mensajes de error (array('clave' => 'mensaje de error',[...]);)
	 * @param string $valor Nuevo texto para el mensaje de error
	 * @return boolean
	 */
	public function setErrorMensaje($clave = false, $valor = false){
		if($clave === false){ //reinicio de mensajes
			$this->aErrorMensajes = array(
				'FormazoProc::valida_2' => '<em>%s</em> tiene caracter obligatorio',
				'FormazoProc::valida_3' => '</em>%s</em> debe ser una dirección de email correcta',
				'FormazoProc::valida_4' => '<em>%s</em> debe ser entero (númerico)',
				'FormazoProc::valida_5' => '</em>%s</em> debe ser una fecha correcta con el formato </em>%s</em>',
				'FormazoProc::valida_6' => '</em>%s</em> debe ser un documento válido',
				'FormazoProc::valida_7' => '<em>%s</em> debe ser numérico',
				'FormazoProc::valida_8' => '</em>%s</em> debe ser una URL correcta'
			);
			return true;
		}

		$aMensajes = array();

		if(!is_array($clave)) $aMensajes[$clave] = $valor;
		else $aMensajes = $clave;

		foreach($aMensajes as $c => $v){
			$this->aErrorMensajes[$c] = $v;
		}
		return true;
	}

	/**
	 * Recoge y prepara los campos que formaran el formulario
	 *
	 * @since 2011-09-14
	 * @param string $nombre Nombre del formulario
	 * @return boolean
	 */
	public function setFormNombre($nombre = false){
		$this->sFormNombre = empty($nombre)?(isset($this->aVariablesRequest['frmFormName']) && $this->aVariablesRequest['frmFormName'] != ''?$this->aVariablesRequest['frmFormName']:false):$nombre;

		return true;
	}

	/**
	 * Recupera valores de campos, para rellenarlos (por ejemplo)
	 *
	 * @since 2015-05-17
	 * @param string $campo Nombre del campo
	 * @return mixed Si se pide un campo devuelve su valor (null si no se encuentra el nombre del campo), si no se devuelve un array con todos los valores
	 */
	public function getValor($campo = null){
		if(empty($campo))
			return $this->aVariablesRequest;
		elseif(!empty($this->aVariablesRequest[$campo])){
			return $this->aVariablesRequest[$campo];
		}

		return null;
	}

	/**
	 * Procesar los datos del formulario
	 *
	 * @param string $nombreForm Nombre del formulario
	 * @return boolean
	 */
	private function setRequest(){
		//recoge GET o POST segun el que se haya utilizado para llegar a esta pagina
		if($_SERVER['REQUEST_METHOD'] == 'GET') $this->aVariablesRequest = &$_GET;
		elseif($_SERVER['REQUEST_METHOD'] == 'POST') $this->aVariablesRequest = &$_POST;
		else $this->aVariablesRequest = array(); //TODO recoger $_REQUEST?

		return true;
	}

	/**
	 * Recoge y valida los datos del formulario
	 *
	 * @todo validaciones de "multiple"
	 * @param string $campo Nombre de campo del formulario
	 * @param string $validacion Validación
	 * @param string $obligatorio Obligatorio
	 * @return boolean
	 */
	private function valida($campo, $validacion = '-', $obligatorio = 0){
		$bRet = true;

		//obligatorio
		if($obligatorio == 1){
			if(!isset($this->aVariablesRequest[$campo]) || $this->aVariablesRequest[$campo] == ''){
				$this->setError(__METHOD__.'_2', $campo);
				$bRet = false;
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
					if(!$this->validaEmail($sTempPost)){
						$this->setError(__METHOD__.'_3', $campo);
						$bRet = false;
					}
					break;
				case 'entero':
					if(!is_numeric($sTempPost)){
						$this->setError(__METHOD__.'_4', $campo);
						$bRet = false;
					}
					break;
				case 'fecha':
					$formato = 'yyyy-mm-dd';
					if(!$this->validaFecha($sTempPost, $formato)){
						$this->setError(__METHOD__.'_5', $campo, $formato);
						$bRet = false;
					}
					break;
				case 'nif':
					if($this->validaNif($sTempPost) <= 0){
						$this->setError(__METHOD__.'_6', $campo);
						$bRet = false;
					}
					break;
				case 'numerico':
					if(!is_numeric($sTempPost)){
						$this->setError(__METHOD__.'_7', $campo);
						$bRet = false;
					}
					break;
				case 'telefono':
					break;
				case 'url':
					if(!$this->validaUrl($sTempPost)){
						$this->setError(__METHOD__.'_8', $campo);
						$bRet = false;
					}
					break;
				default:
					//no se hace validacion
			}

			$this->aCampos[$campo]['valor'] = $sTempPost;
		}

		return $bRet;
	}

	/**
	 * Comprobar la validez de direcciones Email
	 *
	 * @param string $value Valor a evaluar
	 * @return boolean
	 */
	private function validaEmail($value){
		$bRet = true;

		$bRet = preg_match('/\A([-a-zA-Z0-9_.])+\@(([-a-zA-Z0-9])+\.)+([a-zA-Z0-9]{2,4})+\Z/i', $value);

		return $bRet;
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
	 * @param string $value The variable being evaluated.
	 * @param string $format Format of the date. Any combination of mm, dd, yyyy with single character separator between.
	 * @return boolean
	 */
	private function validaFecha($value, $format = 'yyyy-mm-dd'){
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
	 * @param string $cif CIF/NIF/NIE
	 * @return integer
	 */
	private function validaNif($cif){
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
	 * Comprobar la validez de URL's
	 *
	 * @param string $value Valor a evaluar
	 * @return boolean
	 */
	private function validaUrl($value){
		$bRet = true;

		$bRet = preg_match('~\A(?:(?#
protocolo)(https?\:\/\/|ftps?\:\/\/)?(?#
dominio)([\w\.\-]{1,}\.\w{2,6})?(?#
puerto)(\:\d{1,5})?(?#
directorios)(\/[\w;@%=+$\.\-]{1,})*(?#
fichero)(\/[\w+$\.\-]*\.\w*)?(?#
get)(\?([a-z0-9+$_\.\-]&?)*)?(?#
ancla)(#[a-z_\.\-][a-z0-9+$_\.\-]*)?)\Z~i', $value);

		return $bRet;
	}

	/**
	 * Verifica el token de seguridad enviado con el formulario
	 *
	 * @todo verificar funcionamiento
	 * @todo comprobar si ip, sesion y user_agent se establecen correctamente cuando se envia por ajax o se redirecciona el procesado
	 * @since 2012-09-14
	 * @param boolean $session Controla datos relacionados con sesion (true) o no
	 * @return boolean
	 */
	private function verificaToken($session = true){
		$sToken = empty($this->aVariablesRequest['tkn' . $this->sFormNombre]) ? null : $this->aVariablesRequest['tkn' . $this->sFormNombre];

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

		return !(empty($sToken) || md5($token) !== $sToken);
	}

}
