<?php
# Copyright (C) 2015 José M. Carnero <jm_carnero@sargazos.net>
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
 * Generador de formularios
 * Clase generadora de formularios HTML.
 * Genera un formulario completo (entre "<form>" y "</form>", incluidos), con todo lo necesario para hacerlo funcionar, a partir de un array que defina que campos y con que caracteristicas.
 * La validacion javascript se apoya en los nombres de clases css, es completamente externa a la clase (no asi la validacion cuando se procesa en el servidor),
 * debe encontrarse en la seccion "head" (html) de la pagina:
<script type="text/javaScript" src="JaSper.js"></script>
<script type="text/javaScript" src="JaSper_formazo.js"></script>
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

$oForm = new Formazo();
$oForm->setAction('accion.php');
$oForm->setFormNombre('ejemplo');
$oForm->setSubmit('Enviar ejemplo');
$oForm->pinta($aCampos);
 * \endcode
 *
 * @author José M. Carnero
 * @version 5
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Formazo
 */
class Formazo {

	/**
	 * Accion del formulario (url que recoge y procesa los datos).
	 *
	 * @see Formazo::setAction($action)
	 * @var string
	 */
	private $sAction;

	/**
	 * Elementos del formulario
	 *
	 * clave -> nombre (mismos que la tabla donde lanzar la SQL)
	 *  'tipo' -> tipo de cada elemento; tipos: ('checkbox', 'group', 'heading', 'hidden', 'multiple', 'password', 'radio', 'select', 'text', 'textarea'), siendo cada elemento del array el nombre del tag o de tipo de INPUT, excepto "group" que crea agrupaciones de elementos (todos aquellos que tengan 'group' => id_grupo) y "heading" que solo crea cabeceras (titulos de secciones o similar); se ignoran los elementos sin tipo
	 *  'validacion' -> validacion, tipo de datos que contendra; validado por javascript; la cadena se pondra directamente en el atributo "class", asi permite varios tipos de validacion para el mismo campo y es completamente externa a la clase
	 *  'obligatorio' -> not null; 0 (opcional), 1 (obligatorio), string (llamada a una funcion que controle su obligatoriedad, tanto en creacion de formulario como en procesado, la cadena pasada es el nombre de la funcion -sin parentesis-); pasar "0" si se necesitan los siguientes y no es obligatorio
	 *  'longitud' -> longitud maxima (input text, input password, textarea), si =0 -> solo lectura (disabled="disabled"); longitud maxima en caso de ficheros (se pasa en bytes); opcional
	 *  'titulo' -> titulo (humano) del campo
	 *  'valor' -> valor del campo; en caso de "SELECT" el valor es el que toma por defecto
	 *  'valores' -> array con los valores para los "OPTION" o para radioButtons, siendo la clave el valor del "OPTION" y [0] su texto; en select y multiple si cualquier valor es un array se convierte en agrupado (la clave sera la etiqueta del agrupado y el array los options contenidos, en el mismo formato que cualquier option)
	 *  'group' -> id del grupo al que pertenece; si vacio no pertenece a ningun grupo
	 *  'separador' -> solo para campos "multiple", guarda el separador de la lista de valores
	 *  'modo' => modo de representacion del elemento, para "select" puede ser [vacio|select] (select normal) o [radio] (las opciones del select se pintaran como radio buttons, para "multiple" puede ser [nada|multiple] (select multiple normal) o [checkbox] (las opciones del select multiple se pintaran como checkbox); para ficheros: si 'validacion' => 'fichero', 'modo' puede contener los tipos de ficheros permitidos como lista de extensiones separada por comas, ej: 'modo' => 'jpg,png,tiff'
	 *  'rutaFichero' => ruta fisica donde guardar el fichero (para campos file)
	 *  'rutaUrl' => URL donde encontrar el fichero (para campos file) para previsualizarlo
	 *  'sustituir' => solo para ficheros, si true elimina el fichero anterior (el recibido en la propiedad 'valor'), si false crea un nuevo nombre en caso de existir un fichero con el nombre pasado
	 *  //'formato' => solo para campos con validacion fecha, formato de la fecha a validar, ej.: yyyy-mm-dd (por defecto) //de momento no permite hora
	 *  'thumbnail' => debe tener como valor ancho y alto del thumbnail (100x120, ej), solo para imagenes
	 *
	 * @var array
	 * @see Formazo::setCampos($campos), Formazo::getCampos()
	 */
	private $aCampos;

	/**
	 * Clases CSS para los elementos del formulario:
	 * 	label -> todos los label,
	 * 	disabled -> deshabilitado,
	 * Clases de las filas:
	 * 	'primera' (primera fila),
	 * 	'impar' (filas impares),
	 * 	'par' (filas pares),
	 * 	'ultima' (ultima fila),
	 * Clases para validaciones:
	 * 	[ver Formazo::setCss]
	 *
	 * @see Formazo::setCss($clave, $valor)
	 * @since 2012-12-24
	 * @var array
	 */
	protected $aCss;

	/**
	 * Identificador del formulario
	 *
	 * @var string
	 * @see Formazo::getFormNombre(), Formazo::setFormNombre($formName)
	 */
	protected $sFormNombre;

	/**
	 * Formulario resultado; asociativo
	 *
	 * El inicio del formulario se llama "$this->aFormulario['inicio']", el final "$this->aFormulario['final']" y los elementos por su nombre; cada elemento contiene un array asociativo, donde (excepto si se indica lo contrario todos los elementos son necesarios para construir un formulario funcional)
	 * -> "$this->aFormulario['btnSubmit']" es el boton de submit,
	 * -> "$this->aFormulario['elementos']['nombre_elemento_formulario']['texto']" es el titulo que acompana al elemento,
	 * -> "$this->aFormulario['elementos']['nombre_elemento_formulario']['campo']" es el elemento propiamente dicho
	 * -> y "$this->aFormulario['elementos']['nombre_elemento_formulario']['file']" es el fichero (imagen reducida o referencia a fichero) que acompana a los elementos "file", si existe
	 *
	 * @var array
	 */
	public $aFormulario;

	/**
	 * Metodo del formulario "get" o "post" (por defecto).
	 *
	 * @var string
	 */
	private $sMethod;

	/**
	 * En este array se recoge el de GET o POST, segun el que se haya utilizado para llegar a esta pagina; si no se queda con un array vacio
	 *
	 * @var array
	 */
	private $aRequest;

	/**
	 * Valor (texto) del boton de submit
	 *
	 * @var string
	 * @see Formazo::setSubmit($submit)
	 */
	private $sSubmit;

	/**
	 * Indice de tabulacion.
	 *
	 * @todo posibilidad de "compartirlo" entre varios formularios
	 * @var integer
	 */
	public static $iTabIndex;

	/**
	 * Constructor
	 */
	function __construct(){
		if(empty(self::$iTabIndex)) self::$iTabIndex = 1; //tabindex estatico

		$this->sAction = $_SERVER['PHP_SELF']; //por defecto llama a la misma pagina
		$this->aCampos = array();
		$this->setCss();
		$this->sFormNombre = '';
		$this->aFormulario = array();
		$this->sMethod = 'post';
	}

	/**
	 * Destructor
	 */
	function __destruct(){
		//parent::__destruct();
	}

	/**
	 * Agrupa los elementos y devuelve la cadena total
	 *
	 * @access protected
	 * @param array $elementos Elementos preparados en pintado
	 * @return string
	 */
	protected function agrupaElementos($elementos){
		$ret = '';

		foreach($elementos as $clave => $valor){
			if(isset($valor['g'])){
				$ret .= $valor[0];
				$ret .= $this->agrupaElementos($valor['g']);
				$ret .= $valor[1];
			}
			else{
				$ret .= $valor[0];
				$ret .= $valor[1];
			}
		}

		return $ret;
	}

	/**
	 * Calcula CRC (16) para los datos pasados
	 *
	 * @param $data Cadena de texto o numero
	 * @return string
	 */
	private function crc16($data){
		$crc = 0xFFFF;
		for ($i = 0; $i < strlen($data); $i++){
			$x = (($crc >> 8) ^ ord($data[$i])) & 0xFF;
			$x ^= $x >> 4;
			$crc = (($crc << 8) ^ ($x << 12) ^ ($x << 5) ^ $x) & 0xFFFF;
		}
		return $crc;
	}

	/**
	 * Crea el formulario a partir del array de campos que se le pasa
	 *
	 * @param array $campos Campos del formulario
	 * @return boolean
	 */
	public function crea($campos = array()){
		$this->aFormulario = array(); //para crear se limpia el previo //TODO comprobar que mas hay que limpiar antes de crear

		$this->setCampos($campos);
		if(empty($this->aCampos)){
			trigger_error('No hay campos para crear el formulario', E_USER_NOTICE);
			return false;
		}

		$this->setFormNombre(); //nombre del formulario

		//inicia formulario
		if(empty($this->sSubmit)) $this->sAction = '#';
		$this->aFormulario['inicio']['campo'] = '<form id="frm' . $this->sFormNombre . '" action="' . $this->sAction . '" method="' . $this->sMethod . '" class="' . $this->aCss['namespace'] . '">' . "\n";
		$this->aFormulario['inicio']['texto'] = '';

		//crear formulario
		foreach($this->aCampos as $clave => $valor){
			if(empty($valor['tipo'])) continue; //ignora los elementos que no tengan "tipo"; permite leer selectivamente arrays comunes para este y el generador de listados

			$oElemento = null;

			//NOTA si cambian los tipos cambiar los que se ignoran en FormazoProc::setCampos()
			switch(strtolower($valor['tipo'])){
				case 'checkbox':
					$oElemento = new FormazoCheckbox($clave, $valor);
					break;
				case 'file':
					//poner "enctype"
					if(strpos($this->aFormulario['inicio']['campo'], 'enctype') === false){
						//header('Content-Disposition: form-data; name="frm'.$this->sFormNombre.'"');
						$this->aFormulario['inicio']['campo'] = '<form enctype="multipart/form-data"' . substr($this->aFormulario['inicio']['campo'], 5);
					}
					$oElemento = new FormazoFile($clave, $valor);
					break;
				case 'group':
					$oElemento = new FormazoGroup($clave, $valor);
					break;
				case 'hidden':
					$oElemento = new FormazoHidden($clave, $valor);
					break;
				case 'multiple': //select_multiple
					$oElemento = new FormazoMultiple($clave, $valor);
					break;
				case 'password': //text, pero se duplica
					$oElemento = new FormazoPassword($clave, $valor);
					break;
				case 'radio':
					$oElemento = new FormazoRadio($clave, $valor);
					break;
				case 'select': //select_one
					$oElemento = new FormazoSelect($clave, $valor);
					break;
				case 'text':
					$oElemento = new FormazoText($clave, $valor);
					break;
				case 'textarea':
					$oElemento = new FormazoTextarea($clave, $valor);
					break;
				default:
					trigger_error('Tipo de elemento ("' . $valor['tipo'] . '") no reconocido', E_USER_WARNING);
					return false;
			}

			$oElemento->setFormNombre($this->sFormNombre);
			$oElemento->setCss($this->aCss);

			$aElemento = $oElemento->getElemento();

			$this->aFormulario['elementos'][$clave]['texto'] = $aElemento['texto'];
			$this->aFormulario['elementos'][$clave]['campo'] = $aElemento['campo'];
			if(!empty($aElemento['file']))
				$this->aFormulario['elementos'][$clave]['file'] = $aElemento['file'];
		}
		if(empty($this->aFormulario['elementos'])){
			trigger_error('No se han podido procesar los campos', E_USER_ERROR);
			return false;
		}

		$this->aFormulario['btnSubmit']['campo'] = '';
		if(!empty($this->sSubmit)){ //ninguna accion si no hay boton de envio
			$this->aFormulario['btnSubmit']['campo'] = '<input id="btnSubmit' . $this->sFormNombre . '" type="submit" tabindex="' . self::$iTabIndex++ . '" value="' . $this->sSubmit . '" />' . "\n";
		}

		//concluye formulario
		$this->aFormulario['final']['texto'] = $this->aFormulario['btnSubmit']['campo'];
		$this->aFormulario['final']['texto'] .= '<input id="formName' . $this->sFormNombre . '" name="formName" type="hidden" value="' . $this->sFormNombre . "\" />\n"; //nombre del formulario se puede utilizar para identificarlo al procesarlo
		$this->aFormulario['final']['texto'] .= '<input id="tkn' . $this->sFormNombre . '" name="tkn' . $this->sFormNombre . '" type="hidden" value="' . self::token($this->sFormNombre) . "\" />\n"; //token para verificar la integridad del envio

		$this->aFormulario['final']['campo'] = "</form>\n";

		//opcion de aviso antes de salir
		//TODO avisar que campos han cambiado? y solo avisar si realmente ha habido cambios (depende de tener guardado el valor de inicio)

		return true;
	}

	/**
	 * Devuelve los campos que formaran el formulario
	 *
	 * @since 2015-04-30
	 * @return boolean
	 */
	public function getCampos(){
		return $this->aCampos;
	}

	/**
	 * Devuelve el nombre del formulario
	 *
	 * @since 2015-04-30
	 * @return string
	 */
	public function getFormNombre(){
		return $this->sFormNombre;
	}

	/**
	 * Pinta el formulario. Recomendable sobreescribirlo en clases hijas destinadas a dar mejor aspecto al formulario
	 *
	 * @param array $campos Campos del formulario
	 * @param boolean $echo Devuelve el formulario (false) o lo escribe (true, por defecto)
	 * @return string
	 */
	public function pinta($campos = array(), $echo = true){
		if(count($campos) && !count($this->aFormulario))
			$this->crea($campos);

		$sEcho = $this->aFormulario['inicio']['campo'] . $this->aFormulario['inicio']['texto'];
		$i = 0;
		$aEcho = array();
		$sHidden = ''; //agrupa los campos ocultos al final, para evitar problemas de validacion y dibujado en la pagina
		foreach($this->aFormulario['elementos'] as $clave => $valor){
			if($this->aCampos[$clave]['tipo'] != 'hidden'){
				//clases css
				$css = array();
				if(!$i) //primera fila
					$css[] = $this->aCss['primera'];

				if($i%2) //fila impar
					$css[] = $this->aCss['impar'];
				else //fila par
					$css[] = $this->aCss['par'];

				if($i == count($this->aFormulario['elementos']) - 1) //ultima fila
					$css[] = $this->aCss['ultima'];

				if(!empty($css))
					$css = ' class="' . implode(' ', $css) . '"';

				//guardando en una variable para su posterior dibujado
				$aEcho[$clave] = '<p' . $css . '>' . (isset($valor['texto']) ? $valor['texto'] : '') . $valor['campo'] . '</p>';

				//guardando en una variable para su posterior dibujado
				if($this->aCampos[$clave]['tipo'] != 'group'){
					$sApertura = '<p' . $css . '>' . (isset($valor['texto']) ? $valor['texto'] : '') . $valor['campo'];
					$sCierre = '</p>';
				}
				else{
					$sApertura = $valor['texto'];
					$sCierre = $valor['campo'];
				}

				if(!empty($valor['file'])){
					$aEcho[$clave] .= $valor['file'];
					$sCierre .= $valor['file'];
				}

				$i++;

				if(!empty($this->aCampos[$clave]['group'])){
					$aEcho[$this->aCampos[$clave]['group']]['g'][$clave] = array($sApertura, $sCierre);
				}
				else
					$aEcho[$clave] = array($sApertura, $sCierre);
			}
			else $sHidden .= $valor['campo'];
		}

		$sEcho .= $this->agrupaElementos($aEcho);
		$sEcho .= '<p>' . $this->aFormulario['final']['texto'] . $sHidden . '</p>' . $this->aFormulario['final']['campo'];

		if($echo)
			echo $sEcho;
		else
			return $sEcho;
	}

	/**
	 * Asigna la URL de envio del formulario
	 *
	 * @since 2015-05-03
	 * @param string $action Url de envio del formulario
	 * @return string
	 */
	public function setAction($action){
		$this->sAction = $action;
		return $this->sAction;
	}

	/**
	 * Recoge y prepara los campos que formaran el formulario
	 *
	 * @since 2011-07-20
	 * @param array $campos Array con los campos que formaran el formulario
	 * @return boolean
	 */
	public function setCampos($campos = array()){
		if(empty($campos))
			return false;
		elseif(count($campos)){ //origen array
			$this->aCampos = array();

			foreach($campos as $clave => $valor){
				if(isset($valor['tipo'])) //evita procesar campos dedicados exclusivamente al pintado de listados
					$this->aCampos[$clave] = $valor;
			}
		}

		if(empty($this->aCampos)){
			trigger_error('No se puede crear un formulario sin campos', E_USER_ERROR);
			return false;
		}

		//recoge los valores enviados, si es el caso (formulario enviado a la misma pagina, por ejemplo)
		//recoge GET o POST segun el que se haya utilizado para llegar a esta pagina
		if($this->sMethod == 'get')
			$this->aRequest = &$_GET;
		elseif($this->sMethod == 'post')
			$this->aRequest = &$_POST;
		else
			$this->aRequest = array();
		//TODO opcion para recogerlos o no

		foreach($this->aCampos as $clave => $valor){
			if(isset($this->aRequest[$clave]) && $this->aRequest[$clave] != '')
				$this->aCampos[$clave]['valor'] = $this->aRequest[$clave];
		}

		return true;
	}

	/**
	 * Asigna clases CSS
	 *
	 * @since 201-12-24
	 * @access public
	 * @param mixed $clave Clave del array CSS; array completo o false para valores por defecto
	 * @param mixed $clave Clase(s) CSS para la clave
	 * @return boolean
	 */
	public function setCss($clave = false, $clase = false){
		$aCss = array();

		if($clave === false){ //reinicio de clases CSS
			$aCss = array(
				'namespace' => 'Formazo', //pseudo espacio de nombres para todas las clases
				'disabled' => ' disabled', //elementos desabilitados ("disabled")
				'label' => '', 'impar' => 'impar', 'par' => 'par', 'primera' => 'primera', 'ultima' => 'ultima', //pintado de formularios
				'button' => '', 'checkbox' => '', 'file' => '', 'multiple' => '', 'password' => '', 'radio' => '', 'select' => '', 'text' => '', 'textarea' => '', //elementos del formulario
				'bic' => 'bicSwift', 'clave' => 'clave', 'email' => 'email', 'entero' => 'entero', 'fecha' => 'fecha', 'iban' => 'iban', 'nif' => 'nif', 'numerico' => 'numerico', 'obligatorio' => 'obligatorio', 'telefono' => 'telefono', 'url' => 'url' //validaciones
			);

			$this->aCss = $aCss;
		}
		else{
			if(is_array($clave)){
				foreach($clave as $c => $v){
					$this->aCss[$c] = $v;
				}
			}
			else
				$this->aCss[$clave] = $clase;
		}

		return true;
	}

	/**
	 * Asigna el texto del boton de submit del formulario
	 *
	 * @since 2011-08-16
	 * @param string $formName Parte variable del nombre de formulario
	 * @return string
	 */
	public function setFormNombre($formName = ''){
		if($formName != '')
			$this->sFormNombre = $formName;
		elseif($this->sFormNombre == '' && !empty($this->aCampos))
			$this->sFormNombre = $this->crc16(serialize($this->aCampos));

		return $this->sFormNombre;
	}

	/**
	 * Asigna el nombre del formulario (sin "frm")
	 *
	 * @since 2015-05-03
	 * @param string $submit Texto para el boton submit
	 * @return string
	 */
	public function setSubmit($submit){
		$this->sSubmit = $submit;
		return $this->sSubmit;
	}

	/**
	 * Crea el token de seguridad que se enviara con el formulario
	 *
	 * @todo comprobar si ip, sesion y user_agent se establecen correctamente cuando se envia por ajax o se redirecciona el procesado
	 * @since 2012-09-14
	 * @param string $sFormNombre Nombre del formulario
	 * @param boolean $session Controla datos relacionados con sesion (true) o no
	 * @return string
	 */
	public static function token($sFormNombre, $session = true){
		//formulario y procesado deben estar en la misma maquina
		$sSesion = session_id();
		if($session && empty($sSesion)){
			session_start(); //en PHP 4.3.3 o superior puede dar un E_NOTICE si se llama por segunda vez
			$sSesion = session_id();
		}

		$sUserAgent = empty($_SERVER['HTTP_USER_AGENT']) ? '' : $_SERVER['HTTP_USER_AGENT'];

		$sServer = empty($_SERVER['SERVER_ADDR']) ? '' : $_SERVER['SERVER_ADDR'];
		if(!empty($_SERVER['SERVER_PORT']))
			$sServer .= $_SERVER['SERVER_PORT'];
		if(!empty($_SERVER['SERVER_SOFTWARE']))
			$sServer .= $_SERVER['SERVER_SOFTWARE'];
		if(!empty($_SERVER['SERVER_NAME']))
			$sServer .= $_SERVER['SERVER_NAME'];

		$sIp = empty($_SERVER['HTTP_X_FORWARDED_FOR']) ? (empty($_SERVER['REMOTE_ADDR']) ? '999:999:999:999' : $_SERVER['REMOTE_ADDR']) : $_SERVER['HTTP_X_FORWARDED_FOR'];
		//$aCampos = empty($this->aCampos) ? array() : array_keys($this->aCampos);
		$aCampos = array();
		$token = $sFormNombre . '/' . $sSesion . '/' . implode('', $aCampos) . '/' . $sUserAgent . '/' . $sIp . '/' . $sServer;

		return sha1($token);
	}

}

/**************************************
* Clases para elementos de formulario *
**************************************/


/**
 * Clase abstracta de clases para elementos del formulario
 *
 * @todo un valor mas que guarde el valor actual del campo?
 * @todo codificacion
 * @since 2011-06-22
 */
abstract class FormazoElementos {

	protected $sClave; //Nombre del campo

	protected $aCss; //Clases CSS que daran estilo al elemento

	/**
	 * Caracteristicas de los elementos; de cual depende, titulo, valor, etc.
	 *
	 * @var array
	 * @see FormazoElementos::setDatos($datos)
	 */
	protected $aDatos;

	protected $sFormNombre; //Nombre del formulario

	function __construct($clave, $valor){
		$this->sClave = $clave;
		//$aTemp = explode(' ', $this->aFormulario['elementos'][$clave]['propiedades']); //0 tipo, 1 validacion, 2 obligatorio, 3 longitud maxima (=0 -> disabled)
		$this->setDatos($valor);

		$this->sFormNombre = '';
	}

	/**
	 * Elemento del formulario
	 *
	 * @since 2011-06-24
	 * @return array
	 */
	abstract protected function elemento();

	/**
	 * clases CSS, usadas tambien para validaciones
	 *
	 * @since 2015-05-16
	 * @return string
	 */
	protected function getCss(){
		$sCssLabel = '';
		$aCssTag = array();

		if(!empty($this->aCss[$this->aDatos['tipo']])) //los elementos que no llevan tipo son exclusivos para class.listazo.inc
			$aCssTag[] = $this->aCss[$this->aDatos['tipo']];

		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']))
			$aCssTag[] = $this->aCss['obligatorio'];

		if(isset($this->aDatos['validacion'])){
			if(isset($this->aCss[$this->aDatos['validacion']]))
				$aCssTag[] = $this->aCss[$this->aDatos['validacion']];
			else
				$aCssTag[] = $this->aDatos['validacion'];
		}

		$sDisabled = '';
		if(!empty($this->aCss['disabled']) && !empty($this->aDatos['disabled'])){
			$sDisabled = ' disabled="disabled"';
			$aCssTag[] = $this->aCss['disabled'];
		}

		$sCssTag = count($aCssTag) ? ' class="' . implode(' ', $aCssTag) . '"' : '';

		return $sCssTag . $sDisabled;
	}

	/**
	 * Devuelve el elemento formado, validaciones, etc
	 *
	 * @since 2011-06-22
	 * @return array
	 */
	public function getElemento(){
		return $this->elemento();
	}

	/**
	 * Forma la etiqueta label de los elementos
	 *
	 * @since 2015-05-15
	 * @return string
	 */
	protected function getLabel(){
		$sCssLabel = empty($this->aCss['label']) ? '' : ' class="' . $this->aCss['label'] . '"';

		return '<label for="'.$this->sClave.$this->sFormNombre.'"' . $sCssLabel . '>' . $this->aDatos['titulo'] . '</label>';
	}

	/**
	 * Forma la parte "length" o "size" de los elementos que la precisen
	 *
	 * @since 2015-05-15
	 * @return string
	 */
	protected function getLongitud(){
		$sRet = '';

		if(isset($this->aDatos['longitud'])){
			if(is_numeric($this->aDatos['longitud']) && $this->aDatos['longitud'] == 0) //cualquier campo con longitud 0 se deshabilita
				$sRet = ' disabled="disabled"';
			else{
				switch($this->aDatos['tipo']){
					case 'multiple':
					case 'select':
						$sRet = ' size="' . $this->aDatos['longitud'] . '"';
						break;
					case 'text':
					case 'textarea':
						$sRet = ' maxlength="' . $this->aDatos['longitud'] . '"';
						break;
					default;
				}
			}
		}

		return $sRet;
	}

	/**
	 * Asigna el indice de tabulacion
	 *
	 * @since 2015-05-16
	 * @return string
	 */
	public function getTabIndex(){
		$sRet = ' tabindex="' . Formazo::$iTabIndex++ . '"';

		return $sRet;
	}

	/**
	 * Forma la parte "value" de los elementos
	 *
	 * @since 2015-05-15
	 * @param mixed $valor Se usa en alguna subclase donde esta sobreescrito (vease FormazoRadio::getValor)
	 * @return string
	 */
	protected function getValor($valor = false){
		$sRet = ' value="' . $this->aDatos['valor'] . '"';

		return $sRet;
	}

	/**
	 * Transforma la notación "php.ini" para números (como 2M) en enteros (2 * 1024 * 1024, en este caso)
	 *
	 * @access protected
	 * @param string $v Num
	 * @return integer
	 */
	protected function let_to_num($v){
		$l = substr($v, -1);
		$ret = substr($v, 0, -1);
		switch(strtoupper($l)){
			case 'P':
				$ret *= 1024;
			case 'T':
				$ret *= 1024;
			case 'G':
				$ret *= 1024;
			case 'M':
				$ret *= 1024;
			case 'K':
				$ret *= 1024;
				break;
		}

		return $ret;
	}

	/**
	 * Recoge clases CSS.
	 *
	 * @since 2011-06-24
	 * @param array $aCss Clases CSS a aplicar
	 * @return boolean
	 */
	public function setCss($aCss = array()){
		if(empty($aCss))
			return false;

		$this->aCss = $aCss;
		return true;
	}

	protected function setDatos($datos){
		$this->aDatos = empty($datos) ? array() : $datos;

		$this->aDatos['longitud'] = isset($this->aDatos['longitud']) ? $this->aDatos['longitud'] : '';
		$this->aDatos['titulo'] = isset($this->aDatos['titulo']) ? $this->aDatos['titulo'] : '';
		$this->aDatos['valor'] = isset($this->aDatos['valor']) ? $this->aDatos['valor'] : null;

		if(!isset($this->aDatos['separador']))
			$this->aDatos['separador'] = ',';

		return true;
	}

	/**
	 * Recoge el nombre del formulario
	 *
	 * @since 2011-06-24
	 * @param string $sFormNombre Nombre del formulario
	 * @return boolean
	 */
	public function setFormNombre($sFormNombre = ''){
		if(empty($sFormNombre))
			return false;

		$this->sFormNombre = $sFormNombre;
		return true;
	}

}

/**
 * Campo 'input type="checkbox"'
 *
 * @since 2011-06-24
 * @todo implementar estado "indeterminado" (solo se puede controlar con javascript, y envia el mismo valor que inactivo): document.getElementById('checkbox').indeterminate = true;
 */
class FormazoCheckbox extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$aElemento['texto'] = $this->getLabel();
		$aElemento['campo'] = '<input id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '" type="checkbox"' . $this->getTabIndex() . $this->getCss() . $this->getValor() . $this->getLongitud() . ' />';

		return $aElemento;
	}

	protected function getValor($valor = false){
		$sRet = parent::getValor($valor);
		$sRet .= $this->aDatos['valor'] ? ' checked="checked"' : '';

		return $sRet;
	}

}

/**
 * Campo 'input type="file"'
 *
 * @since 2011-06-24
 */
class FormazoFile extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();
		$this->aDatos['rutaFichero'] = empty($this->aDatos['rutaFichero']) ? '.' : $this->aDatos['rutaFichero']; //TODO evita problemas si no se ha puesto ruta en el fichero, pero no comprueba si esta es valida, deberia ser /tmp por defecto?
		$this->aDatos['rutaUrl'] = isset($this->aDatos['rutaUrl']) ? $this->aDatos['rutaUrl'] : $this->aDatos['rutaFichero'];

		$aElemento['texto'] = $this->getLabel();

		//fijar tamano maximo
		//tamano maximo del fichero a subir en bytes (es facil saltarse esto, recontrolar con php en procesado del formulario)
		$iMaxUpload = min($this->let_to_num(ini_get('post_max_size')), $this->let_to_num(ini_get('upload_max_filesize')));
		if(isset($this->aDatos['longitud']) && $this->aDatos['longitud'] > 0){
			$iMaxUpload = min($iMaxUpload, $this->aDatos['longitud']);
		}
		$this->aFormulario['elementos'][$this->sClave]['campo'] = '<input type="hidden" name="MAX_FILE_SIZE" value="' . $iMaxUpload . '" />';
		$aElemento['campo'] = '<input id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '" type="file" data-previewId="img_' . $this->sClave . $this->sFormNombre . '" ' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . ' />'; //data-previewId se usa en javascript para crear una vista previa cuando se cambia de fichero
		$aElemento['campo'] .= '<br />Tama&ntilde;o m&aacute;ximo: ' . $iMaxUpload . ' bytes';

		//muestra la imagen (si lo es), o el nombre del fichero actual
		$aElemento['file'] = '<br /><div id="pre_' . $this->sClave . $this->sFormNombre . '"><a href="' . $this->aDatos['rutaUrl'] . rawurldecode($this->aDatos['valor']) . '" target="_blank">';
		$sExtension = empty($this->aDatos['valor']) ? null : strtolower(substr($this->aDatos['valor'], strrpos($this->aDatos['valor'], '.') + 1));
		switch($sExtension){
			case 'jpg':
			case 'jpeg':
			case 'gif':
			case 'png':
				$aElemento['file'] .= '<img id="img_' . $this->sClave . $this->sFormNombre . '" src="' . $this->aDatos['rutaUrl'] . rawurldecode($this->aDatos['valor']) . '" alt="Imagen actual" />';
				break;
			default:
				$aElemento['file'] .= '<img id="img_' . $this->sClave . $this->sFormNombre . '" src="Imagen preview" alt="' . $this->aDatos['valor'] . '" />'; //TODO esta es la mejor manera de mostrar el valor actual?, usando una imagen inexistente como "placeholder" para la futura preview por javascript?
		}
		$aElemento['file'] .= '</a></div>';

		return $aElemento;
	}

}

/**
 * Agrupacion de elementos en 'fieldset'
 * Observese que "$aElemento['texto']" es la apertura del grupo (apertura de div) y "$aElemento['campo']" es el cierre del grupo
 *
 * @since 2011-06-22
 */
class FormazoGroup extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = $aCssTag = array();

		$aElemento['texto'] = '<fieldset id="' . $this->sClave . $this->sFormNombre . '"' . $this->getCss() . '><legend>' . $this->aDatos['titulo'] . '</legend>';
		$aElemento['campo'] = '</fieldset>';

		return $aElemento;
	}

}

/**
 * Campo 'input type="hidden"'
 *
 * @since 2011-06-22
 */
class FormazoHidden extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$aElemento['texto'] = '';
		$aElemento['campo'] = '<input id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '" type="hidden"' . $this->getValor() . $this->getLongitud() . ' />';

		return $aElemento;
	}

}

/**
 * Campo 'select' multiple,
 * Los valores seleccionados han de ser una lista separada por comas
 * ej. valores con grupos: 'valores' => array('grupo 1' => array('valor 1' => 'texto 1', 'valor 2' => 'texto 2', 'valor 3' => 'texto 3'), 'grupo 2' => array('valor 4' => 'texto 4', 'valor 5' => 'texto 5'))
 *
 * @todo el nombre del campo acaba en "[]" para que PHP pueda recoger todos los valores enviados, pero esto da problemas de validacion; buscar otro sistema, quizas añadiendo [] con javascript antes de enviar el formulario?
 * @since 2011-06-22
 */
class FormazoMultiple extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$this->aDatos['modo'] = (!empty($this->aDatos['modo']) && $this->aDatos['modo'] == 'checkbox') ? 'checkbox' : 'multiple'; //modo de dibujado del select [multiple|checkbox]

		$aElemento['texto'] = $this->getLabel();
		if($this->aDatos['modo'] == 'multiple') $aElemento['campo'] = '<select id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave.'[]" multiple="multiple"' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . '>';
		elseif($this->aDatos['modo'] == 'checkbox') $aElemento['campo'] = '<span id="' . $this->sClave . $this->sFormNombre . '" >';

		//options
		if(isset($this->aDatos['valores']) && count($this->aDatos['valores']) > 0){
			$aTempVal = array();
			if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])){ //array de valores
				$aTempVal = $this->aDatos['valor'];
			}
			elseif(isset($this->aDatos['valor']) && strpos($this->aDatos['valor'], $this->aDatos['separador']) !== false){ //valores seleccionados separados por $this->aDatos['separador']
				$aTempVal = explode($this->aDatos['separador'], $this->aDatos['valor']);
			}
			elseif(isset($this->aDatos['valor']) && strpos($this->aDatos['valor'], $this->aDatos['separador']) === false){ //un solo valor
				$aTempVal[] = $this->aDatos['valor'];
			}

			$aTempVal = array_map('trim', $aTempVal);
			$aTempVal = array_flip($aTempVal); //TODO posibilidad de que algun valor no se pueda usar como clave (por convenciones de nombrado)

			if(count($this->aDatos['valores']) > 0){
				$iCheckCont = 0;
				foreach($this->aDatos['valores'] as $c => $v){
					if($this->aDatos['modo'] == 'multiple'){
						if(is_array($v)){
							$aElemento['campo'] .= '<optgroup label="' . $c . '">';
							foreach($v as $c2 => $v2)
								$aElemento['campo'] .= '<option value="' . $c2 . '"' . (isset($aTempVal[$c2]) ? ' selected="selected"' : '') . '>' . $v2 . '</option>';
							$aElemento['campo'] .= '</optgroup>';
						}
						else
							$aElemento['campo'] .= '<option value="' . $c . '"' . (isset($aTempVal[$c]) ? ' selected="selected"' : '') . '>' . $v . '</option>';
					}
					elseif($this->aDatos['modo'] == 'checkbox'){
						if(is_array($v)){
							$aElemento['campo'] .= '<span>' . $c;
							foreach($v as $c2 => $v2)
								$aElemento['campo'] .= '<input id="c_' . $iCheckCont . $this->sClave . '" name="' . $this->sClave . '[]" type="checkbox"' . $this->getCss() . $this->getLongitud() . ' value="' . $c2 . '"' . (isset($aTempVal[$c2]) ? ' checked="checked"' : '') . ' /><label for="c_' . ($iCheckCont++) . $this->sClave . '">' . $v2 . '</label>';
							$aElemento['campo'] .= '</span>';
						}
						else $aElemento['campo'] .= '<input id="c_' . $iCheckCont . $this->sClave . '" name="' . $this->sClave . '[]" type="checkbox"' . $this->getCss() . $this->getLongitud() . ' value="' . $c . '"' . (isset($aTempVal[$c]) ? ' checked="checked"' : '') . ' /><label for="c_' . ($iCheckCont++) . $this->sClave . '">' . $v . '</label>';
					}
				}
			}
		}

		if($this->aDatos['modo'] == 'multiple') $aElemento['campo'] .= '</select>';
		elseif($this->aDatos['modo'] == 'checkbox') $aElemento['campo'] .= '</span>';

		return $aElemento;
	}

}

/**
 * Campo 'input type="password"'
 *
 * @since 2011-06-22
 */
class FormazoPassword extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$aElemento['texto'] = $this->getLabel();
		$aElemento['campo'] = '<input id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '" type="password"' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . $this->getValor() . ' autocomplete="off" />'; //autocomplete="off" es importante para que firefox no rellene con autocompletado (aunque solo funciona en carga de pagina, no cuando se tabula por los campos)

		return $aElemento;
	}

}

/**
 * Campo 'input type="radio"'
 *
 * @since 2011-06-22
 */
class FormazoRadio extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$aElemento['texto'] = '<span>' . $this->aDatos['titulo'] . '</span><br />';
		$aElemento['campo'] = '';

		//botones
		if(count($this->aDatos['valores']) > 0){
			foreach($this->aDatos['valores'] as $c => $v){
				$aElemento['campo'] .= '<label for="r_' . $c . $this->sFormNombre . '">' . $v . '</label>&nbsp;';
				$aElemento['campo'] .= '<input id="r_' . $c . $this->sFormNombre . '" name="' . $this->sClave . '" type="radio"' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . $this->getValor($c) . ' value="' . $c . '" />';
			}
		}

		return $aElemento;
	}

	protected function getValor($valor = false){
		$sRet = '';
		if($this->aDatos['valor'] == $valor){
			$sRet = ' checked="checked"';
		}

		return $sRet;
	}

}

/**
 * Campo 'select'
 * ej. valores con grupos: 'valores' => array('grupo 1' => array('valor 1' => 'texto 1', 'valor 2' => 'texto 2', 'valor 3' => 'texto 3'), 'grupo 2' => array('valor 4' => 'texto 4', 'valor 5' => 'texto 5'))
 *
 * @since 2011-06-22
 */
class FormazoSelect extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$this->aDatos['modo'] = (!empty($this->aDatos['modo']) && $this->aDatos['modo'] == 'radio') ? 'radio' : 'select'; //modo de dibujado del select [select|radio]

		$aElemento['texto'] = $this->getLabel();
		if($this->aDatos['modo'] == 'select') $aElemento['campo'] = '<select id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '"' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . '>';
		elseif($this->aDatos['modo'] == 'radio') $aElemento['campo'] = '<span id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '">';

		if(isset($this->aDatos['valores']) && count($this->aDatos['valores']) > 0){
			$iRadioCont = 0;
			foreach($this->aDatos['valores'] as $c => $v){
				if($this->aDatos['modo'] == 'select'){
					if(is_array($v)){
						$aElemento['campo'] .= '<optgroup label="' . $c . '">';
						foreach($v as $c2 => $v2) $aElemento['campo'] .= '<option value="' . $c2 . '"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c2) ? ' selected="selected"' : '') . '>' . $v2 . '</option>';
						$aElemento['campo'] .= '</optgroup>';
					}
					else
						$aElemento['campo'] .= '<option value="' . $c . '"' . ((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c) ? ' selected="selected"' : '').'>' . $v . '</option>';
				}
				elseif($this->aDatos['modo'] == 'radio'){
					if(is_array($v)){
						$aElemento['campo'] .= '<span>' . $c;
						foreach($v as $c2 => $v2) $aElemento['campo'] .= '<input id="r_' . $iRadioCont . $this->sClave . '" name="' . $this->sClave . '" type="radio"' . $this->getCss() .$this->getLongitud() . ' value="' . $c2 . '"' . ((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c2) ? ' checked="checked"' : '') . ' /><label for="r_' . ($iRadioCont++) . $this->sClave . '">' . $v2 . '</label>';
						$aElemento['campo'] .= '</span>';
					}
					else $aElemento['campo'] .= '<input id="r_' . $iRadioCont . $this->sClave . '" name="' . $this->sClave . '" type="radio"' . $this->getCss() . $this->getLongitud() . ' value="' . $c . '"' . ((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c) ? ' checked="checked"' : '') . ' /><label for="r_' . ($iRadioCont++) . $this->sClave.'">' . $v . '</label>';
				}
			}
		}

		if($this->aDatos['modo'] == 'select') $aElemento['campo'] .= '</select>';
		elseif($this->aDatos['modo'] == 'radio') $aElemento['campo'] .= '</span>';

		return $aElemento;
	}

}

/**
 * Campo 'input type="text"'
 *
 * @since 2011-06-22
 */
class FormazoText extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$aElemento['texto'] = $this->getLabel();
		$aElemento['campo'] = '<input id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '" type="text"' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . $this->getValor() . ' />';

		return $aElemento;
	}

}

/**
 * Campo 'textarea'
 *
 * @since 2011-06-22
 */
class FormazoTextarea extends FormazoElementos {

	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		$aElemento = array();

		$aElemento['texto'] = $this->getLabel();
		$aElemento['campo'] = '<textarea id="' . $this->sClave . $this->sFormNombre . '" name="' . $this->sClave . '"' . $this->getTabIndex() . $this->getCss() . $this->getLongitud() . ' rows="" cols="">' . $this->getValor() . '</textarea>';

		return $aElemento;
	}

	protected function getValor($valor = false){
		$sRet = $this->aDatos['valor'];

		return $sRet;
	}

}
