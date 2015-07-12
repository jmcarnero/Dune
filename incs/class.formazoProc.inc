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

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta
if(!defined('CLASSES_DIR')) define('CLASSES_DIR', dirname(__FILE__));

/**
 * Clase procesadora de formularios HTML
 * Tanto GET como POST
 *
 * depende de class.formazo.inc para la generacion del token de seguridad: Formazo::token($session)
 *
 * Ejemplo de uso:
 * \code{.php}
$aCampos = array(
'titulo' => array('tipo' => 'text', 'validacion' => '-', 'obligatorio' => '0', 'longitud' => '100', 'titulo' => 'T&iacute;tulo: '),
'texto' => array('tipo' => 'textarea', 'validacion' => '-', 'obligatorio' => '0', 'longitud' => '16000000', 'titulo' => 'Texto: '),
'fichero' => array('tipo' => 'file', 'titulo' => 'Fichero: ', 'validacion' => 'fichero', 'modo' => 'jpg,gif,png'),
'tipoSel' => array('tipo' => 'select', 'titulo' => 'Tipo select: ', 'valores' => array('' => 'ninguno', '0' => 'cero', '1' => 'uno', '2' => 'dos', '3' => 'tres'), 'modo' => 'radio'),
'tipoMul' => array('tipo' => 'multiple', 'titulo' => 'Tipo multiple: ', 'valores' => array('' => 'ninguno', '0' => 'cero', '1' => 'uno', 'doses' => array('2' => 'dos', '2nd' => 'segundo'), '3' => 'tres'), 'valor' => '0,2', 'modo' => 'checkbox'), //select multiple con agrupados y dos valores preseleccionados
'activa' => array('tipo' => 'checkbox', 'titulo' => 'Activa: ')
);

if(!empty($_POST['formName'])){
	include_once("inc/class.formazoProc.inc");

	$oProcFormulario = new FormazoProc($aCampos);

	$aCampos['titulo']['valor'] = $oProcFormulario->getValor('titulo');
	$aCampos['texto']['valor'] = $oProcFormulario->getValor('texto');

	if($oProcFormulario->procesarFormulario($_POST['formName'])){ //validacion correcta
		//var_dump('sql-> ' . $oProcFormulario->getRedatazo);
	}
	else{ //errores
		$this->aDatos['frmError'] = $oProcFormulario->getError(); //problemas de validacion, devolver a la pagina con estos mensajes
	}
}
//else{
//	echo('Ningún formulario a procesar.');
//}
 * \endcode
 *
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
	 * Devuelve los campos, si se recuperan despues del procesado tendran los valores actualizados (como nombres de ficheros subidos)
	 *
	 * @param boolean $bRaw Si true devuelve $this->aCampos tal cual, si false devuelve solo clave => valor
	 * @return array
	 */
	public function getCampos($bRaw = false){
		$aRet = $this->aCampos;

		if(!$bRaw){
			foreach($this->aCampos as $clave => $valor){
				$aRet[$clave] = $valor['valor'];
			}
		}

		return $aRet;
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
	 * Limpia nombres de ficheros.
	 *
	 * Original: Cameron Perry (http://iamcam.wordpress.com/2007/03/20/clean-file-names-using-php-preg_replace/)
	 *
	 * @todo cortar el resultado (255 caracteres?) segun sistema operativo pueden dar problemas los nombres largos
	 * @todo opcion para convertir el nombre a camelCase
	 * @param $nombreFichero Nombre del fichero a limpiar
	 * @return string
	 */
	private function limpiarNombreFichero($nombreFichero){
		$sLimpiarNombreFichero = '';

		if($sLimpiarNombreFichero === false) return($nombreFichero);

		$nombreFichero = html_entity_decode(strip_tags($nombreFichero));
		//$nombreFichero = ucwords($nombreFichero);
		$patron = "/([[:alnum:]_\.-]*)/";
		$nombreFichero = str_replace(str_split(preg_replace($patron, $sLimpiarNombreFichero, $nombreFichero)), $sLimpiarNombreFichero, $nombreFichero);
		//if(strlen($nombreFichero) > 255) $nombreFichero = substr($nombreFichero, 0, 255);

		return $nombreFichero;
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
			trigger_error('El token generado y el recibido no coinciden', E_USER_WARNING);
			return false;
		}


		return $this->procesarValores(); //si false ha habido errores en la recuperacion de valores del formulario
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
					//aqui no se hacen subidas reales, solo se testea la validacion
					if(!$this->subeArchivo($clave, true)){
						$bRet = false; //problemas con la subida de ficheros
					}
					break;
				default:
					trigger_error('El tipo de campo [<em>' . $valor['tipo'] . '</em>] no se reconoce', E_USER_NOTICE);
					$bRet = false;
			}
		}

		//validacion correcta, se suben los ficheros a sus ubicaciones definitivas
		if($bRet){
			foreach($this->aCampos as $clave => $valor){
				if($valor['tipo'] == 'file'){
					if(!$this->subeArchivo($clave)){ //error con "move_uploaded_file"
						$bRet = false; //problemas con la subida de ficheros
					}
				}
			}
		}

		return $bRet;
	}

	/**
	 * Comprueba si existe el nombre del fichero pasado y construye un nombre que no exista
	 *
	 * @param string $fichero Nombre del fichero
	 * @return string
	 */
	private function renombraArchivo($fichero, $sRutaFicheros){
		if(file_exists($sRutaFicheros . $fichero)){
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

			return $this->renombraArchivo($nombre . $num . '.' . $ext, $sRutaFicheros);
		}

		return($fichero);
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
		/*else{ //vacio, origen desconocido o ya asignado en formazo::setCampos()
			if(empty($this->aCampos)){
				trigger_error('No se puede crear un formulario sin campos', E_USER_ERROR);
				return false;
			}
		}*/

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
				'FormazoProc::valida_bic' => '</em>%s</em> debe ser un código BIC/SWIFT',
				//'FormazoProc::valida_clave' => '</em>%s</em> debe ser una clave correcta',
				'FormazoProc::valida_email' => '</em>%s</em> debe ser una dirección de email correcta',
				'FormazoProc::valida_entero' => '<em>%s</em> debe ser entero (númerico)',
				'FormazoProc::valida_fecha' => '</em>%s</em> debe ser una fecha correcta con el formato </em>%s</em>',
				'FormazoProc::valida_iban' => '</em>%s</em> debe ser un código IBAN',
				'FormazoProc::valida_nif' => '</em>%s</em> debe ser un documento válido',
				'FormazoProc::valida_numerico' => '<em>%s</em> debe ser numérico',
				//'FormazoProc::valida_telefono' => '<em>%s</em> debe ser un número de teléfono correcto',
				'FormazoProc::valida_url' => '</em>%s</em> debe ser una URL correcta'
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
	 * Asigna la ruta (por omision) donde se guardaran los ficheros que se suban,
	 * solo se usa en ficheros que no llevan su propia ruta de guardado
	 *
	 * @since 2011-09-14
	 * @param string $ruta Ruta de guardado
	 * @return boolean
	 */
	private function setRutaFicheros($ruta = false){
		if(empty($ruta) && $ruta != 0){
			trigger_error('No se ha indicado la ruta para subir ficheros', E_USER_ERROR);
			$ruta = null;
		}
		if(!is_writable($ruta)){
			trigger_error('No se pueden subir ficheros al directorio [' . $ruta . ']', E_USER_ERROR);
			$ruta = null;
		}
		$ruta = rtrim($ruta, '\\/').'/'; //pone el ultimo /

		return $ruta;
	}

	/**
	 * Sube ficheros
	 *
	 * @param string $campo Clave del campo que contiene el fichero a subir
	 * @param boolean $bTest Si true no sube el fichero realmente, solo comprueba que puede subirse; esto es util para verificar la validacion
	 * @return boolean
	 */
	private function subeArchivo($campo, $bTest = false){
		$bTest = !empty($bTest);

		if(isset($_FILES[$campo]) && is_uploaded_file($_FILES[$campo]['tmp_name'])){
			$sRutaFicheros = $this->setRutaFicheros($this->aCampos[$campo]['rutaFichero']);
			if(empty($sRutaFicheros)) //ruta de subida invalida
				return false;

			//validando tipo de fichero
			if(isset($_FILES[$campo]) && !empty($this->aCampos[$campo]['validacion']) && $this->aCampos[$campo]['validacion'] == 'fichero' && isset($this->aCampos[$campo]['modo'])){
				$tipos = ',' . strtolower($this->aCampos[$campo]['modo']) . ',';
				$fileExt = strrpos($_FILES[$campo]['name'], '.');
				if($fileExt === false) $fileExt = '';
				else $fileExt = strtolower(substr($_FILES[$campo]['name'], $fileExt + 1));

				if(strpos($tipos, ',' . $fileExt . ',') === false){
					trigger_error('Tipo de fichero no permitido [' . $fileExt . ']', E_USER_WARNING);
					return false;
				}
			}

			//limpiando y preparando nombre
			$sTempArchNombre = $_FILES[$campo]['name'];
			$sTempArchNombre = $this->limpiarNombreFichero($sTempArchNombre);

			if(empty($this->aCampos[$campo]['pisar'])){ //si esta opcion esta a true se elimina el fichero anterior, si existe; "move_uploaded_file" lo hace
				$sTempArchNombre = $this->renombraArchivo($sTempArchNombre, $sRutaFicheros);
			}

			if($bTest) //modo verificacion, no se hace subida real
				return true;

			//mover el archivo a su ubicacion definitiva
			if(@move_uploaded_file($_FILES[$campo]['tmp_name'], $sRutaFicheros . $sTempArchNombre)){
				$this->aCampos[$campo]['valor'] = rawurlencode($sTempArchNombre); //guardado del nombre del fichero subido
			}
			else{
				trigger_error('No se ha podido subir el fichero [' . $sTempArchNombre . '] a la ruta [' . $sRutaFicheros . ']', E_USER_WARNING);
				return false;
			}
		}

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
				case 'bic': //codigo bancario BIC/SWIFT
					if(!$this->validaBicSwift($sTempPost)){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
						$bRet = false;
					}
					break;
				case 'clave':
					break;
				case 'email':
					if(!$this->validaEmail($sTempPost)){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
						$bRet = false;
					}
					break;
				case 'entero':
					if($this->validaNumero($sTempPost, 'int')){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
						$bRet = false;
					}
					break;
				case 'fecha':
					$formato = 'yyyy-mm-dd';
					if(!$this->validaFecha($sTempPost, $formato)){
						$this->setError(__METHOD__ . '_' . $validacion, $campo, $formato);
						$bRet = false;
					}
					break;
				case 'iban': //codigo bancario IBAN
					if(!$this->validaIban($sTempPost)){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
						$bRet = false;
					}
					break;
				case 'nif':
					if($this->validaNif($sTempPost) <= 0){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
						$bRet = false;
					}
					break;
				case 'numerico':
					if($this->validaNumero($sTempPost)){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
						$bRet = false;
					}
					break;
				case 'telefono':
					break;
				case 'url':
					if(!$this->validaUrl($sTempPost)){
						$this->setError(__METHOD__ . '_' . $validacion, $campo);
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
	 * Comprobar la validez de codigos de banco tipo BIC/SWIFT
	 * Structure:
The latest edition is ISO 9362:2009 (dated 2009-10-01). The SWIFT code is 8 or 11 characters, made up of:
4 letters: Institution Code or bank code.
2 letters: ISO 3166-1 alpha-2 country code
2 letters or digits: location code
if the second character is "0", then it is typically a test BIC as opposed to a BIC used on the live network. if the second character is "1", then it denotes a passive participant in the SWIFT network if the second character is "2", then it typically indicates a reverse billing BIC, where the recipient pays for the message as opposed to the more usual mode whereby the sender pays for the message. 3 letters or digits: branch code, optional ('XXX' for primary office)
	 *
	 * @since 2015-05-31
	 * @param string $value Valor a evaluar
	 * @return boolean
	 */
	private function validaBicSwift($value){
		$bRet = true;

		$bRet = preg_match('/^[a-z]{6}[0-9a-z]{2}([0-9a-z]{3})?$/i', trim($value));

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

		if(function_exists('filter_var')) //filtrado PHP (>= 5.2.0) nativo
			$bRet = filter_var($value, FILTER_VALIDATE_EMAIL);
		else
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
	 * @todo incluir la validacion de HH:mm:ss
	 * @param string $value The variable being evaluated.
	 * @param string $format Format of the date. Any combination of mm, dd, yyyy with single character separator between.
	 * @return boolean
	 */
	private function validaFecha($value, $format = 'yyyy-mm-dd'){
		/*$iFecha = (int) @strtotime($sFecha);
		if($iFecha < 1){*/

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
					$day = substr($value,strpos($format, 'd'),2);
					$month = substr($value,strpos($format, 'm'),2);
					$year = substr($value,strpos($format, 'y'),4);

					if(@checkdate($month, $day, $year)) return true;
				}
			}
		}

		return false;
	}

	/**
	 * Comprobar la validez de codigos de banco tipo IBAN
	 *
	 * @since 2015-05-31
	 * @param string $sIban Valor a evaluar
	 * @return boolean
	 */
	private function validaIban($sIban){
		$sIban = trim($sIban);
		$bRet = true;

		if(!empty($sIban)){
			// remove spaces and to upper case
			$sIban = strtoupper(preg_replace('/([^[:alnum:]_])/', '', $sIban));
			$leadingZeroes = true;
			$ibancheckdigits = '';

			// check the country code and find the country specific format
			$countrycode = substr($sIban, 0, 2);
			$bbancountrypatterns = array(
					'AD' => '\d{8}[\dA-Z]{12}', 'AE' => '\d{3}\d{16}', 'AL' => '\d{8}[\dA-Z]{16}', 'AT' => '\d{16}', 'AZ' => '[\dA-Z]{4}\d{20}',
					'BA' => '\d{16}', 'BE' => '\d{12}', 'BG' => '[A-Z]{4}\d{6}[\dA-Z]{8}', 'BH' => '[A-Z]{4}[\dA-Z]{14}', 'BR' => '\d{23}[A-Z][\dA-Z]',
					'CH' => '\d{5}[\dA-Z]{12}', 'CR' => '\d{17}', 'CY' => '\d{8}[\dA-Z]{16}', 'CZ' => '\d{20}',
					'DE' => '\d{18}', 'DK' => '\d{14}', 'DO' => '[A-Z]{4}\d{20}',
					'EE' => '\d{16}', 'ES' => '\d{20}',
					'FI' => '\d{14}', 'FO' => '\d{14}', 'FR' => '\d{10}[\dA-Z]{11}\d{2}',
					'GB' => '[A-Z]{4}\d{14}', 'GE' => '[\dA-Z]{2}\d{16}', 'GI' => '[A-Z]{4}[\dA-Z]{15}', 'GL' => '\d{14}', 'GR' => '\d{7}[\dA-Z]{16}', 'GT' => '[\dA-Z]{4}[\dA-Z]{20}',
					'HR' => '\d{17}', 'HU' => '\d{24}',
					'IE' => '[\dA-Z]{4}\d{14}', 'IL' => '\d{19}', 'IS' => '\d{22}', 'IT' => '[A-Z]\d{10}[\dA-Z]{12}',
					'KW' => '[A-Z]{4}[\dA-Z]{22}', 'KZ' => '\d{3}[\dA-Z]{13}',
					'LB' => '\d{4}[\dA-Z]{20}', 'LI' => '\d{5}[\dA-Z]{12}', 'LT' => '\d{16}', 'LU' => '\d{3}[\dA-Z]{13}', 'LV' => '[A-Z]{4}[\dA-Z]{13}',
					'MC' => '\d{10}[\dA-Z]{11}\d{2}', 'MD' => '[\dA-Z]{2}\d{18}', 'ME' => '\d{18}', 'MK' => '\d{3}[\dA-Z]{10}\d{2}', 'MR' => '\d{23}', 'MT' => '[A-Z]{4}\d{5}[\dA-Z]{18}', 'MU' => '[A-Z]{4}\d{19}[A-Z]{3}',
					'NL' => '[A-Z]{4}\d{10}', 'NO' => '\d{11}',
					'PK' => '[\dA-Z]{4}\d{16}', 'PL' => '\d{24}', 'PS' => '[\dA-Z]{4}\d{21}', 'PT' => '\d{21}',
					'RO' => '[A-Z]{4}[\dA-Z]{16}', 'RS' => '\d{18}',
					'SA' => '\d{2}[\dA-Z]{18}', 'SE' => '\d{20}', 'SI' => '\d{15}', 'SK' => '\d{20}', 'SM' => '[A-Z]\d{10}[\dA-Z]{12}',
					'TN' => '\d{20}', 'TR' => '\d{5}[\dA-Z]{17}',
					'VG' => '[\dA-Z]{4}\d{16}'
			);

			$bbanpattern = empty($bbancountrypatterns[$countrycode]) ? null : $bbancountrypatterns[$countrycode];

			if(empty($bbanpattern)){ //pais desconocido
				$bRet = false;
			}
			else{
				if(preg_match('/^[A-Z]{2}\d{2}' . $bbanpattern . '$/', $sIban)){
					// now check the checksum, first convert to digits
					$ibancheck = mb_substr($sIban, 4, mb_strlen($sIban)) . mb_substr($sIban, 0, 4);
					$charAt = -1;
					for($i = 0; $i < mb_strlen($ibancheck); $i++){
						$charAt = mb_substr($ibancheck, $i, 1);
						if($charAt !== '0'){
							$leadingZeroes = false;
						}
						if(!$leadingZeroes){
							$ibancheckdigits .= strpos('0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ', $charAt);
						}
					}

					$cRest = '';
					//calculate the result of: $ibancheckdigits % 97
					for($p = 0; $p < mb_strlen($ibancheckdigits); $p++){
						$cChar = mb_substr($ibancheckdigits, $p, 1);
						$cOperator = $cRest . $cChar;
						$cRest = $cOperator % 97;
					}
					$bRet = ($cRest === 1);
				}
				else {
					$bRet = false; // invalid country specific format
				}
			}
		}

		return $bRet;
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
	 * Comprobar la validez de numeros
	 *
	 * @param string $value Valor a evaluar
	 * @param string $value Tipo de valor numerico: bool, float, int, [null - cualquier numero]
	 * @return boolean
	 */
	private function validaNumero($value, $tipo = null){
		$bRet = true;

		/*if(function_exists('filter_var')) //filtrado PHP (>= 5.2.0) nativo
			$bRet = filter_var($value, FILTER_VALIDATE_INT); //FILTER_VALIDATE_BOOLEAN, FILTER_VALIDATE_FLOAT, FILTER_VALIDATE_INT
		else*/
			$bRet = is_numeric($value);

		return $bRet;
	}

	/**
	 * Comprobar la validez de URL's
	 *
	 * @param string $value Valor a evaluar
	 * @return boolean
	 */
	private function validaUrl($value){
		$bRet = true;

		if(function_exists('filter_var')) //filtrado PHP (>= 5.2.0) nativo
			$bRet = filter_var($value, FILTER_VALIDATE_URL);
		else
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
	 * @see Formazo::token($session)
	 * @param boolean $session Controla datos relacionados con sesion (true) o no
	 * @return boolean
	 */
	private function verificaToken($session = true){
		@include_once CLASSES_DIR . '/class.formazo.inc';

		$sToken = false;
		if(class_exists('Formazo')){
			$sToken = Formazo::token($this->sFormNombre, $session);
		}

		return $sToken;
	}

}
