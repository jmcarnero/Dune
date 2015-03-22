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

/**
 * Generador de formularios
 * Clase generadora de formularios HTML
 *
 * @author José M. Carnero
 * @version 4
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Formazo
 */

/*
 * Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta.
 */
if(!defined('CLASSFORM_DIR')) define('CLASSFORM_DIR', dirname(__FILE__));

/**
 * Clase generadora de formularios HTML.
 * Genera un formulario completo (entre "<form>" y "</form>", incluidos), con todo lo necesario para hacerlo funcionar, a partir de un array que defina que campos y con que caracteristicas.
 * Requiere de una clase de procesado de datos, "class.Formazo_proc.inc" (con una pagina que la implemente).
 * La validacion javascript se apoya en los nombres de clases css, es completamente externa a la clase (no asi la validacion cuando se procesa en el servidor), aunque la clase provee codigo necesario para campos ligados y dependientes,
 * debe encontrarse en la seccion "head" (html) de la pagina, un ejemplo:
<script type="text/javaScript" src="JaSper.js"></script>
<script type="text/javaScript" src="JaSper_formazo.js"></script>
 * Ejemplo de uso:
 * \code{.php}
include_once("class.formazo.inc");

$aCampos = array(
'titulo' => array('tipo' => 'text', 'validacion' => '-', 'obligatorio' => '0', 'longitud' => '100', 'titulo' => 'T&iacute;tulo: '),
'texto' => array('tipo' => 'textarea', 'validacion' => '-', 'obligatorio' => '0', 'longitud' => '16000000', 'titulo' => 'Texto: '),
'fichero' => array('tipo' => 'file', 'titulo' => 'Fichero: ', 'validacion' => 'fichero', 'extra' => array('modo' => 'jpg,gif,png')),
'tipo' => array('tipo' => 'select', 'titulo' => 'Tipo: ', 'valores' => array('' => 'ninguno', '0' => 'cero', '1' => 'uno', '2' => 'dos', '3' => 'tres')),
//'tipo' => array('tipo' => 'multiple', 'titulo' => 'Tipo: ', 'valores' => array('' => 'ninguno', '0' => 'cero', '1' => 'uno', 'doses' => array('2' => 'dos', '2nd' => 'segundo'), '3' => 'tres'), 'valor' => '0,2'), //select multiple con agrupados y dos valores preseleccionados
'activa' => array('tipo' => 'checkbox', 'titulo' => 'Activa: ')
);

$oForm = formazo_sel('tabla'); //tambien se puede $oForm = new Formazo();
//$oForm->sRutaFicheros = 'img/';
//$oForm->sQueryTabla = 'tabla_ej';
//$oForm->sRowId = 'id'; //nombre de id a actualizar/borrar
//$oForm->sRowIdVal = $sId; //valor de id a actualizar/borrar
$oForm->sAction = 'proc_formazo.php';

$oForm->creaFormulario($aCampos);
echo($oForm->getJavaScript());

$oForm->pintaFormulario();
//echo($oForm->aError);
 * \endcode
 *
 * PHP 5
 *
 * @todo por ahora los nombres de los campos han de ser distintos; importante si se crea mas de un formulario en la misma pagina
 * @todo controlar obligatoriedad en campos dependientes
 * @author José Manuel Carnero
 * @version 4
 */
class Formazo{

	/**
	 * En este array se recoge el de GET o POST, segun el que se haya utilizado para llegar a esta pagina; si no se queda con un array vacio
	 *
	 * @var array
	 * @access private
	 */
	private $aVariablesRequest;

	/**
	 * Indice de tabulacion.
	 *
	 * @todo posibilidad de "compartirlo" entre varios formularios
	 * @var integer
	 * @access private
	 */
	private static $iTabIndex;

	/**
	 * Lista de textos visibles por los usuarios finales usados en funciones javascript y html
	 *
	 * @var array
	 * @see Formazo::getMetodosMensaje($clave),Formazo::setMetodosMensaje($clave, $valor)
	 * @access private
	 */
	private $aMetodosMensajes;

	/**
	 * Accion del formulario (url que recoge y procesa los datos)
	 *
	 * @var string
	 * @access public
	 */
	public $sAction;

	/**
	 * Campo oculto con la accion a realizar en base de datos: upd (UPDATE), ins (INSERT), del(DELETE), busc(SELECT, busqueda) tambien define que SQL devolvera este objeto
	 *
	 * @var string
	 * @see Formazo::setActionHide($sActionHide)
	 * @access protected
	 */
	protected $sActionHide;

	/**
	 * Permite tener un solo formulario para actualizacion y borrado; guarda el titulo para el boton de borrado extra o nada (actualizacion y borrado por separado)
	 *
	 * @var string
	 * @access public
	 */
	public $sActionUpdDel;

	/**
	 * Metodo del formulario "get" o "post" (por defecto)
	 *
	 * @var string
	 * @access public
	 */
	public $sMethod;

	/**
	 * Booleano, si debe aparecer los botones en la parte de arriba del formulario
	 *
	 * @var boolean
	 * @access public
	 */
	public $bBtmTop;

	/**
	 * Elementos del formulario
	 * (si un elemento no tiene 'tipo' se ignora, ya que puede ser un array de elementos compartido con class.listado.inc)
	 *
	 * key -> nombre (mismos que la tabla donde lanzar la SQL)
	 * 'tipo' -> tipo de cada elemento; tipos: ('checkbox', 'group', 'heading', 'hidden', 'multiple', 'password', 'radio', 'select', 'text', 'textarea'), siendo cada elemento del array el nombre del tag o de tipo de INPUT, excepto "group" que crea agrupaciones de elementos (todos aquellos que tengan 'group' => id_grupo) y "heading" que solo crea cabeceras (titulos de secciones o similar); se ignoran los elementos sin tipo
	 * 'validacion' -> validacion, tipo de datos que contendra; validado por javascript; la cadena se pondra directamente en el atributo "class", asi permite varios tipos de validacion para el mismo campo y es completamente externa a la clase
	 * 'obligatorio' -> not null; 0 (opcional), 1 (obligatorio), string (llamada a una funcion que controle su obligatoriedad, tanto en creacion de formulario como en procesado, la cadena pasada es el nombre de la funcion -sin parentesis-); pasar "0" si se necesitan los siguientes y no es obligatorio
	 * 'longitud' -> longitud maxima (input text, input password, textarea), si =0 -> solo lectura (disabled="disabled") y valor actual en campo hidden; longitud maxima en caso de ficheros (se pasa en bytes); opcional
	 * 'titulo' -> titulo (humano) del campo
	 * 'valor' -> valor del campo; en caso de "SELECT" el valor es el que toma por defecto
	 * 'valores' -> array con los valores para los "OPTION" o para radioButtons, siendo la clave el valor del "OPTION" y [0] su texto; en select y multiple si cualquier valor es un array se convierte en agrupado (la clave sera la etiqueta del agrupado y el array los options contenidos, en el mismo formato que cualquier option)
	 * 'group' -> id del grupo al que pertenece; si vacio no pertenece a ningun grupo
	 * 'hint' -> pista (tooltip) sobre el elemento
	 * 'help' -> asociativo con texto de ayuda del elemento, clave indica posicion, valor el texto de ayuda
	 * 	 0 => antes del elemento (antes de label), 1 => despues del elemento
	 * 'ligado' -> para select enlazado (normal o multiple), contiene la id del select del que depende el actual (mas detalles en el metodo)
	 * 'depende' -> campos dependientes de otro, contiene la id del campo del que depende
	 * 	 'id' => campo del que depende
	 * 	 'val' => valor que debe tener el campo 'id' para que este se muestre
	 * 	 'hid' => 'line' muestra/oculta toda la linea (por defecto), 'tag' muestra/oculta solo el tag (input) afectado)
	// * 'repetir' -> repetir elemento o grupo (como tener un text con la talla de unos zapatos, repetir la caja text para poner otra talla y asi sucesivamente), tiene como valor un identificador que agrupa a los elementos a repetir con ese mismo identificador permitiendo repetir un solo elemento o un grupo de ellos; su valor tambien puede ser el identificador (no array) si no se requiere maximo y/o minimo
	// * 	 'id' => identificador de grupo repetido
	// * 	 'min' => minimo de repeticiones (0 por defecto)
	// * 	 'max' => maximo de repeticiones (indefinido por defecto)
	 * 'val_submit' -> condiciones extra para el evento de submit del formulario; en la forma 'funcion(parametros) ', debe devolver cierto o falso y solo tener un espacio final (sustituir cualquier otro espacio y caracter problematico con $this->limpiaJs())
	 * 'separador' -> solo para campos "multiple", guarda el separador de la lista de valores
	 * 'css' -> clase CSS que llevara el elemento (en caso de radio lo llevarian todos)
	 * 'extra' -> array asociativo, clave indica posicion y tipo del elemento
	 * 	 0 => izquierda de label, 1 => derecha de label, 2 => izquierda de elemento, 3 => derecha del elemento
	 * 	 'p0' => antes de 'texto', 'p1' => despues de 'campo', estas ultimas opciones se usan en postprocesado, como dibujado en tablas; estas suman el valor al elemento ya formado (como una imagen que sirva para llevar a una ayuda especifica del campo); debe ser html completo, no se trata, simplemente se suma; no se pasa al procesado
	 * 	 'modo' => modo de representacion del elemento, en caso de "group" define el tipo de grupo ([repeat|tab|block|row|fieldset]), en caso de "heading" define si es una cabecera o un simple parrafo (tendra como valor el tipo de etiqueta html que lo contendra, o nada), para "select" puede ser [vacio|select] (select normal) o [radio] (las opciones del select se pintaran como radio buttons, para "multiple" puede ser [nada|multiple] (select multiple normal) o [checkbox] (las opciones del select multiple se pintaran como checkbox)
	 * 'dataSource' => origen de datos para campos select //debe ser una llamada a funcion prefijada con "php:" o "js:" (javascript), si no tiene prefijo se asume que es javascript //el primer parametro debe ser '$id$' (incluyendo las comillas), que sera sustituido por la id del campo select que rellenara; asi, el formato de la funcion debe ser "nombre_funcion('$id$'[, otros_parametros]);"//si es llamada a php espera que se devuelva un select completo, si es js solo sus options (aunque al pasarse el id es sencillo substituirlo entero)
	 *
	 * @var array
	 * @see Formazo::setCampos($campos),Formazo::getCampos()
	 * @access protected
	 */
	protected $aCampos;

	/**
	 * Clases CSS para los elementos del formulario: label -> todos los label, cada tipo de elemento segun su nombre, dis -> deshabilitado, o en borrado(funciona como sufijo a la clase normal), obligatorio -> obligatorios, fecha -> en inputs (text) validados como fecha
	 * Clases de las filas (grupo label + elemento + ... - extras), asociativo: 'titulo' (primera fila), 'fimpar' (filas impares), 'fpar' (filas pares), 'ultima' (ultima fila)
	 *
	 * @see Formazo::getCss(),Formazo::setCss($clave, $valor)
	 * @since 2012-12-24
	 * @var array
	 * @access protected
	 */
	protected $aCss;

	/**
	 * Mensajes de error
	 *
	 * @var string
	 * @see Formazo::getError(),Formazo::setError()
	 * @access private
	 */
	private $aError;

	/**
	 * Mensajes de error la clase, a fin de que puedan ser traducidos/modificados
	 * Las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas, si es necesario poner un % se pondra como %%
	 *
	 * @var array
	 * @see Formazo::setErrorMensaje($clave, $valor),Formazo::getErrorMensaje($clave)
	 * @access private
	 */
	private $aErrorMensajes;

	/**
	 * Identificador del formulario se le añadira el prefijo "frm" ("name = frm".$this->sFormNombre)
	 * Se usa tambien para la variable de sesion que guarda la query SQL, asi se pueden enviar varias querys por pagina, se envia como campo oculto
	 * Tambien concatenada a "id" de cada elemento del formulario, asi es posible construirlos con los mismos valores de nombre, que se haran distintos al construir el formulario
	 * Ya que se pasa como valor oculto, posteriormente (en el procesado) puede retirarse si se necesitan los nombres originales de los campos
	 *
	 * @var string
	 * @see getFormNombre(),setFormNombre($formName)
	 * @access protected
	 */
	protected $sFormNombre;

	/**
	 * Formulario resultado; asociativo
	 * El inicio del formulario se llama "$this->aFormulario['frmInicio']", el final "$this->aFormulario['frmFinal']" y los elementos por su nombre; cada elemento contiene un array asociativo, donde (excepto si se indica lo contrario todos los elementos son necesarios para construir un formulario funcional);
	 * -> "$this->aFormulario['frmBtnSubmit']" es el boton de submit,
	 * -> "$this->aFormulario['frmValidar']" contiene las validaciones javascript del formulario, debe escribirse en la seccion "head" de la pagina,
	 * -> "$this->aFormulario['frmElementos']['nombre_elemento_formulario']['texto']" es el titulo que acompana al elemento,
	 * -> "$this->aFormulario['frmElementos']['nombre_elemento_formulario']['campo']" es el elemento propiamente dicho
	 * -> y "$this->aFormulario['frmElementos']['nombre_elemento_formulario']['file']" es el fichero (imagen reducida o referencia a fichero) que acompana a los elementos "file", si existe
	 *
	 * @var array
	 * @access public
	 */
	public $aFormulario;

	/**
	 * Ruta del fichero que se usara como icono para "hints" (mensajes de ayuda de campos)
	 *
	 * @var string
	 * @see Formazo::setHintIco($hint)
	 * @access private
	 */
	private $sHintIco;

	/**
	 * Codigo Javascript creado por las funciones Formazo::fDepende() y Formazo::fLigado()
	 *
	 * @var array
	 * @see Formazo::getJavaScript();
	 * @access private
	 */
	private $aJavaScript;

	/**
	 * Tabla donde insertar/actualizar datos
	 *
	 * @var string
	 * @access public
	 */
	public $sQueryTabla;

	/**
	 * Ruta (absoluta o relativa) donde se encuentran los ficheros (para mostrarlos en edicion/borrado)
	 *
	 * @todo controlar si la ruta de los ficheros esta puesta en la base de datos ("file_exists"?)
	 * @var string
	 * @see Formazo::setRutaFicheros($ruta)
	 * @access public
	 */
	protected $sRutaFicheros;

	/**
	 * Lista de funciones permitidas para rellenar selects
	 *
	 * @var array
	 * @see Formazo::setSourceFuncs($funcs)
	 * @access protected
	 */
	protected $aSourceFuncs;

	/**
	 * Valor (texto) del boton de submit
	 *
	 * @var string
	 * @access public
	 */
	public $sSubmit;

	/**
	 * Tiempos consumidos en la generacion del formulario, tanto total como de elementos, ...
	 *
	 * @var array
	 * @see Formazo::getTiempos()
	 * @access protected
	 */
	protected $aTiempos;

	/**
	 * Constructor
	 */
	function __construct(){
		$this->aError = array();
		$this->setErrorMensaje(); //inicializacion de los mensajes de error
		$this->aTiempos = array('total' => 0); //medicion de tiempo

		//$this->iTabIndex = 0;
		if(!isset(self::$iTabIndex)) self::$iTabIndex = 1; //tabindex estatico

		$this->sAction = $_SERVER['PHP_SELF']; //por defecto llama a la misma pagina
		$this->setActionHide('ins'); //por defecto insercion
		$this->sActionUpdDel = ''; //por defecto formularios para actualizacion y borrado son independientes
		$this->sMethod = 'post';
		$this->sFormNombre = '';
		$this->sSubmit = 'Enviar';
		$this->aCampos = array();
		$this->setRutaFicheros('./'); //por defecto la misma ruta donde se ejecute
		$this->aFormulario = array();
		$this->sQueryTabla = '';
		$this->bBtmTop = false;
		$this->aJavaScript = array();
		$this->setCss();
		$this->sHintIco = '#';

		$this->setMetodosMensaje(); //inicializacion de los textos visibles de la clase
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

		return($ret);
	}

	/**
	 * Calcula CRC (16) para los datos pasados
	 *
	 * @access private
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
	 * @access public
	 * @param array $campos Campos del formulario
	 * @return boolean
	 */
	public function creaFormulario($campos = array()){
		//medicion de tiempo
		$this->aTiempos['creaFormulario'] = $this->microtimeSeg(true);

		$this->aFormulario = $this->aJavaScript = array(); //para crear se limpia el previo //TODO comprobar que mas hay que limpiar antes de crear

		$this->setCampos($campos);
		if(empty($this->aCampos)) return(false);

		//nombre del formulario
		$this->setFormNombre();

		//inicia formulario
		if(empty($this->sSubmit)) $this->sAction = '#';
		$this->aFormulario['frmInicio']['campo'] = '<form id="frm'.$this->sFormNombre.'" action="'.$this->sAction.'" method="'.$this->sMethod.'">'."\n";
		$this->aFormulario['frmInicio']['texto'] = '';

		$aTempRepeat = array(); //guarda fragmentos de elementos repetidos

		//crear formulario
		foreach($this->aCampos as $clave => $valor){
			if(empty($valor['tipo'])) continue; //ignora los elementos que no tengan "tipo"; permite leer selectivamente arrays comunes para este y el generador de listados

			//medicion de tiempo
			$this->aTiempos['elementos'][$clave] = $this->microtimeSeg(true);

			if(isset($valor['extra'])) $this->aFormulario['frmElementos'][$clave]['extra'] = $valor['extra']; //se guarda el componente extra por si es necesario en postprocesado (como en dibujado en tabla)

			if(isset($valor['depende'])) $valor['depende']['val_op'] = isset($this->aCampos[$valor['depende']['id']]['valor'])?$this->aCampos[$valor['depende']['id']]['valor']:null; //pasando valor de dependencia

			//evita mensajes vacios, como los que puede producir tinymce (mucho html y nada de contenido)
			if(!empty($valor['hint']) && preg_replace('/\s|&nbsp;/i', '', strip_tags($valor['hint'])) == '') $valor['hint'] = '';

			//sin validaciones en borrado
			if(!empty($this->sSubmit) && $this->sActionHide == 'del') $valor['validacion'] = '';

			//NOTA si cambian los tipos cambiar los que se ignoran en Formazo_proc::setCampos()
			switch(strtolower($valor['tipo'])){
				case 'checkbox':
					$oElemento = new FormazoCheckbox($clave, $valor);
					break;
				case 'file':
					//poner "enctype"
					if(strpos($this->aFormulario['frmInicio']['campo'], 'enctype') === false)
						//header('Content-Disposition: form-data; name="frm'.$this->sFormNombre.'"');
						$this->aFormulario['frmInicio']['campo'] = '<form enctype="multipart/form-data"'.substr($this->aFormulario['frmInicio']['campo'], 5);
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
					$this->setError('Formazo::creaFormulario', $valor['tipo']);
					return(false);
			}

			//propiedades del elemento a construir
			$oElemento->setErrorMensaje($this->getErrorMensaje());
			$oElemento->setMetodosMensaje($this->getMetodosMensaje());

			$oElemento->setActionHide($this->sActionHide);
			$oElemento->setCss($this->aCss);
			$oElemento->setExtra($this->extra($clave));
			$oElemento->setFormNombre($this->sFormNombre);
			$oElemento->setSourceFuncs($this->aSourceFuncs);
			$oElemento->setTabIndex(self::$iTabIndex);
			$oElemento->setRutaFicheros($this->sRutaFicheros);
			$oElemento->setHintIco($this->sHintIco);

			$aElemento = $oElemento->getElemento();
			if($oElemento->getError()){
				$this->aError[] = $oElemento->getError();
				return false;
			}
			if(!empty($aElemento)) self::$iTabIndex++;

			$this->fDepende($clave);
			$this->fLigado($clave);
			if(!empty($valor['validacion'])){
				if($valor['validacion'] == 'fecha' && isset($valor['extra']['modo'])) $this->aJavaScript[] = 'var format_'.$clave.$this->sFormNombre.' = "'.$valor['extra']['modo'].'";'."\n";
				if($valor['validacion'] == 'fichero' && isset($valor['extra']['modo'])) $this->aJavaScript[] = 'var file_'.$clave.$this->sFormNombre.' = "'.$valor['extra']['modo'].'";'."\n";
			}
			//javascript para reconstruir elementos repetidos
			if(isset($valor['valor']) && is_array($valor['valor']) && (strtolower($this->aCampos[$valor['group']]['extra']['modo']) == 'list' || strtolower($this->aCampos[$valor['group']]['extra']['modo']) == 'repeat' || strtolower($this->aCampos[$valor['group']]['extra']['modo']) == 'row')){
				array_shift($valor['valor']); //el primer valor del array se usa para el primer elemento (que esta clase)
				$aTempRepeat[$valor['group'].$this->sFormNombre][] = array_merge(array($clave), $valor['valor']);
			}


			$this->aFormulario['frmElementos'][$clave]['texto'] = $aElemento['texto'];
			$this->aFormulario['frmElementos'][$clave]['campo'] = $aElemento['campo'];
			if(!empty($aElemento['help'])) $this->aFormulario['frmElementos'][$clave]['help'] = $aElemento['help'];
			if(!empty($aElemento['hint'])) $this->aFormulario['frmElementos'][$clave]['hint'] = $aElemento['hint'];
			if(!empty($aElemento['file'])) $this->aFormulario['frmElementos'][$clave]['file'] = $aElemento['file'];
			if(strtolower($valor['tipo']) == 'password' && !empty($valor['validacion'])){ //confirmacion de clave
				$this->aFormulario['frmElementos'][$clave.'2']['texto'] = $aElemento[2]['texto'];
				$this->aFormulario['frmElementos'][$clave.'2']['campo'] = $aElemento[2]['campo'];
			}
			if($this->getError()) $this->aError[] = $this->getError();

			//medicion de tiempo
			$this->aTiempos['elementos'][$clave] = $this->microtimeSeg(true) - $this->aTiempos['elementos'][$clave];
		}
		if(empty($this->aFormulario['frmElementos'])){
			$this->setError('Formazo::creaFormulario2');
			return(false);
		}

		//preparando javascript para zonas repetidas
		if(!empty($aTempRepeat)){
			$temp = '';
			foreach($aTempRepeat as $clave => $valor){
				$temp .= '{\''.$clave.'\':[{';
				$aTemp = array();
				for($i=0;$i<count($valor);$i++){
					for($j=1;$j<count($valor[$i]);$j++){
						$aTemp[$j] = (isset($aTemp[$j])?$aTemp[$j].',':'')."'".$valor[$i][0]."':'".$valor[$i][$j]."'";
					}
				}
				$temp .= implode("},{", $aTemp).'}]},';
			}
			$temp = trim($temp, ',');
			$this->aJavaScript[] = 'var repeat_'.$this->sFormNombre.' = '.$temp.";\n";
		}

		$this->aFormulario['frmBtnSubmit'] = $this->aFormulario['frmBtnSubmitDel'] = $this->aFormulario['frmBtnLimpia'] = $this->aFormulario['frmBtnCancelar'] = array('campo' => null);

		if(!empty($this->sSubmit)){ //ninguna accion si no hay boton de envio
			if($this->sActionHide == 'del') $this->aFormulario['frmBtnSubmit']['campo'] = '<input class="'.$this->aCss['button'].' frm'.ucfirst($this->sActionHide).'" type="submit" value="'.$this->sSubmit.'" />'."\n";
			else{
				$this->aFormulario['frmBtnSubmit']['campo'] = '<input id="frmBtnSubmit'.$this->sFormNombre.'" class="'.$this->aCss['button'].' frm'.ucfirst($this->sActionHide).'" type="submit" tabindex="'.self::$iTabIndex++.'" value="'.$this->sSubmit.'" />'."\n";
				if($this->sActionHide == 'upd' && $this->sActionUpdDel != ''){ //boton de borrado en formularios de actualizacion
					$this->aFormulario['frmBtnSubmitDel']['campo'] = '<input id="frmButtonUpdDel'.$this->sFormNombre.'" class="'.$this->aCss['button'].' frmSubmitDel" type="button" tabindex="'.self::$iTabIndex++.'" value="'.$this->sActionUpdDel.'" />'."\n";
				}
			}

			$this->aFormulario['frmBtnLimpia']['campo'] .= (($this->sActionHide == 'busc')?'<input id="frmButtonLimpia'.$this->sFormNombre.'" class="'.$this->aCss['button'].' frmLimpia" type="button" tabindex="'.self::$iTabIndex++.'" value="'.$this->aMetodosMensajes[__METHOD__.'_3'].'" />':'');

			//cancelar formulario (volver atras en el historial)
			$this->aFormulario['frmBtnCancelar']['campo'] = '<input id="frmButtonCancelar'.$this->sFormNombre.'" class="'.$this->aCss['button'].' frmCancelar" type="button" tabindex="'.self::$iTabIndex++.'" value="'.$this->aMetodosMensajes[__METHOD__.'_4'].'" />'."\n";
		}

		//concluye formulario
		$this->aFormulario['frmFinal']['campo'] = $this->aFormulario['frmBtnSubmit']['campo'].$this->aFormulario['frmBtnSubmitDel']['campo'].$this->aFormulario['frmBtnLimpia']['campo'].$this->aFormulario['frmBtnCancelar']['campo'];
		//$this->aFormulario['frmFinal']['campo'] .= '<input name="hidCampos" type="hidden" value="'.implode(' ', array_values($this->aCampos))."\" />\n"; //lista de campos y tipos del formulario; para inserciones en base de datos los nombres del formulario han de ser los mismos que los campos de la tabla
		$this->aFormulario['frmFinal']['campo'] .= '<input id="frmFormName'.$this->sFormNombre.'" name="frmFormName" type="hidden" value="'.$this->sFormNombre."\" />\n"; //nombre del formulario se utiliza para enviar la query a la pagina de procesado, mediante variable de sesion
		$this->aFormulario['frmFinal']['campo'] .= '<input id="frmAccion'.$this->sFormNombre.'" name="frmAccion" type="hidden" value="'.$this->sActionHide."\" />\n"; //accion del formulario sobre la base de datos
		$this->aFormulario['frmFinal']['campo'] .= '<input id="frmTkn'.$this->sFormNombre.'" name="frmTkn" type="hidden" value="'.$this->generaToken()."\" />\n"; //token para verificar la integridad del envio
		$this->aFormulario['frmFinal']['campo'] .= "</form>\n";

		$this->aFormulario['frmFinal']['texto'] = '';

		//opcion de aviso antes de salir
		//TODO avisar que campos han cambiado? y solo avisar si realmente ha habido cambios (depende de tener guardado el valor de inicio)

		//medicion de tiempo
		$tempTime = $this->microtimeSeg(true);
		$this->aTiempos['creaFormulario'] = $tempTime - $this->aTiempos['creaFormulario'];
		$this->aTiempos['total'] += $this->aTiempos['creaFormulario'];

		return(true);
	}

	/**
	 * Prepara los datos de la informacion extra de los campos
	 *
	 * @access private
	 * @param string $clave Clave del campo
	 * @return integer
	 */
	private function extra($clave){
		//colocacion de componente extra
		$aExtra = array('', '', '', '');
		if(!empty($this->aCampos[$clave]['extra'])){
			if(is_array($this->aCampos[$clave]['extra'])){
				if(isset($this->aCampos[$clave]['extra'][0]) && $this->aCampos[$clave]['extra'][0] != '') $aExtra[0] = $this->aCampos[$clave]['extra'][0];
				if(isset($this->aCampos[$clave]['extra']['0']) && $this->aCampos[$clave]['extra']['0'] != '') $aExtra[0] = $this->aCampos[$clave]['extra']['0'];
				if(isset($this->aCampos[$clave]['extra'][1]) && $this->aCampos[$clave]['extra'][1] != '') $aExtra[1] = $this->aCampos[$clave]['extra'][1];
				if(isset($this->aCampos[$clave]['extra']['1']) && $this->aCampos[$clave]['extra']['1'] != '') $aExtra[1] = $this->aCampos[$clave]['extra']['1'];
				if(isset($this->aCampos[$clave]['extra'][2]) && $this->aCampos[$clave]['extra'][2] != '') $aExtra[2] = $this->aCampos[$clave]['extra'][2];
				if(isset($this->aCampos[$clave]['extra']['2']) && $this->aCampos[$clave]['extra']['2'] != '') $aExtra[2] = $this->aCampos[$clave]['extra']['2'];
				if(isset($this->aCampos[$clave]['extra'][3]) && $this->aCampos[$clave]['extra'][3] != '') $aExtra[3] = $this->aCampos[$clave]['extra'][3];
				if(isset($this->aCampos[$clave]['extra']['3']) && $this->aCampos[$clave]['extra']['3'] != '') $aExtra[3] = $this->aCampos[$clave]['extra']['3'];
			}
			else $aExtra[3] = $this->aCampos[$clave]['extra'];
		}
		return($aExtra);
	}

	/**
	 * Construye el codigo JavaScript para los campos con dependencias
	 *
	 * @access private
	 * @since 2011-03-15
	 * @param string $campo Clave del campo
	 * @return boolean
	 */
	private function fDepende($clave){
		$sufijo = '';
		if(!empty($this->aCss['dis']) && ($this->sActionHide=='del' || (isset($this->aCampos[$clave]['longitud']) && $this->aCampos[$clave]['longitud'] == 0)))
			$sufijo = $this->aCss['dis'];

		if(!empty($this->aCampos[$clave]['depende']) && $sufijo == ''){
			$sTipo = isset($this->aCampos[$this->aCampos[$clave]['depende']['id']]['tipo'])?strtolower($this->aCampos[$this->aCampos[$clave]['depende']['id']]['tipo']):''; //tipo de elemento del campo del que depende $clave

			if(empty($this->aCampos[$clave]['depende']['hid']) || $this->aCampos[$clave]['depende']['hid'] != 'tag') $idTag = 'Fil';

			//$this->aJavaScript['depende'][] = '\''.$clave.$this->sFormNombre.$idTag.'\':[\''.($sTipo == 'radio'?'@':'#').$this->aCampos[$clave]['depende']['id'].$this->sFormNombre.'\',\''.$this->aCampos[$clave]['depende']['val'].'\']';
			if(isset($this->aCampos[$this->aCampos[$clave]['depende']['id']]['extra']['modo']) && $this->aCampos[$this->aCampos[$clave]['depende']['id']]['extra']['modo'] == 'radio') $id = '#'.$this->aCampos[$clave]['depende']['id'].$this->sFormNombre.' input[name="'.$this->aCampos[$clave]['depende']['id'].'"]';
			elseif(isset($this->aCampos[$this->aCampos[$clave]['depende']['id']]['extra']['modo']) && $this->aCampos[$this->aCampos[$clave]['depende']['id']]['extra']['modo'] == 'checkbox') $id = '#'.$this->aCampos[$clave]['depende']['id'].$this->sFormNombre.' input[name="'.$this->aCampos[$clave]['depende']['id'].'[]"]';
			else $id = ($sTipo == 'radio'?'@':'#').$this->aCampos[$clave]['depende']['id'].$this->sFormNombre; //campos select y radio (normales)

			if(empty($this->aJavaScript['depende'][$id])) $this->aJavaScript['depende'][$id] = '[\''.$clave.$this->sFormNombre.$idTag.'\',\''.$this->aCampos[$clave]['depende']['val'].'\']';
			else $this->aJavaScript['depende'][$id] .= ',[\''.$clave.$this->sFormNombre.$idTag.'\',\''.$this->aCampos[$clave]['depende']['val'].'\']';
			//TODO dependencia para select pintados como checkbox o radio; "id del div que ha sustituido al select input:name[select]" (o lo que corresponda para cada elemento sustitutivo de options) [lo que corresponda del elemento dependiente]
		}

		return(true);
	}

	/**
	 * Construye el codigo javascript para los campos ligados a otros campos (ej. select que cambian todos sus elementos en funcion del valor de otro select)
	 * Si los valores mostrados estan ligados a otro select, las etiquetas de grupo deben ser las mismas que las mostradas en el select del que dependen.
	 * ej:
'select_normal' => array('tipo' => 'select', 'titulo' => 'select: ', 'valores' => array('' => 'ninguno', '0' => 'grupo cero', '1' => 'grupo uno', '2' => 'grupo dos', '3' => 'grupo tres')),
'select_multiple' => array('tipo' => 'multiple', 'titulo' => 'multiple: ', 'valores' => array('' => array('nada'), '0' => array('0' => 'cero', 'c' => 'calabaza'), '1' => array('1' => 'uno', '1o' => 'primero'), '2' => array('2' => 'dos', '2nd' => 'segundo'), '3' => array('3' => 'tres', 'tr' => 'ternario', '3o' => 'tercero')), 'ligado' => 'select_normal'),
	 *
	 * @access private
	 * @since 2011-01-22
	 * @param string $campo Clave del campo
	 * @return boolean
	 */
	private function fLigado($clave){
		if(!isset($this->aCampos[$clave]['ligado']) || $this->aCampos[$clave]['ligado'] == '') return(false);

		if(empty($this->aFormulario['frmElementos'][$clave]['frmJs'])) $this->aFormulario['frmElementos'][$clave]['frmJs'] = '';

		$aTempEti = $aTempVal = array();
		foreach($this->aCampos[$clave]['valores'] as $valor){
			if(!is_array($valor)) $valor = array($valor);
			$aTempVal[] .= "['".implode("', '", array_keys($valor))."']";
			$aTempEti[] .= "['".implode("', '", $valor)."']";
		}
		$this->aJavaScript[] = 'var etiquetas_'.$clave.$this->sFormNombre.' = new Array('.implode(",\n", $aTempEti).');'."\n";
		$this->aJavaScript[] = 'var valores_'.$clave.$this->sFormNombre.' = new Array('.implode(",\n", $aTempVal).');'."\n";
		$this->aJavaScript['ligado'][] = '\''.$clave.$this->sFormNombre.'\':\''.$this->aCampos[$clave]['ligado'].$this->sFormNombre.'\'';
		/*//if($this->sActionHide != 'ins'){ //cargar grupo de elementos seleccionados
			$this->aJavaScript[] = '$(\'#'.$clave.$this->sFormNombre.'\').llena_select(
			valores_'.$clave.$this->sFormNombre.'[document.getElementById(\''.$this->aCampos[$clave]['ligado'].$this->sFormNombre.'\').selectedIndex],
			etiquetas_'.$clave.$this->sFormNombre.'[document.getElementById(\''.$this->aCampos[$clave]['ligado'].$this->sFormNombre.'\').selectedIndex], \''.$this->aCampos[$clave]['valor'].'\');';
		//}*/

		return(true);
	}

	/**
	 * Crea el token de seguridad que se enviara con el formulario
	 *
	 * @todo comprobar si ip, sesion y user_agent se establecen correctamente cuando se envia por ajax o se redirecciona el procesado
	 * @since 2012-09-14
	 * @access private
	 * @param boolean $session Controla datos relacionados con sesion (true) o no
	 * @return boolean
	 */
	private function generaToken($session = true){
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

		return(md5($token));
	}

	/**
	 * Devuelve los campos que formaran el formulario
	 *
	 * @since 2011-10-03
	 * @access public
	 * @return boolean
	 */
	public function getCampos(){
		return($this->aCampos);
	}

	/**
	 * Muestra la clase CSS asignada a una clave
	 * Si no se pasa ninguna clave se muestran todos
	 *
	 * @since 2012-12-24
	 * @access public
	 * @param $clave Clave de la clase CSS, si se omite (o false) muestra el array completo
	 * @return string
	 */
	public function getCss($clave = false){
		if($clave === false) return($this->aCss);
		return($this->aCss[$clave]);
	}

	/**
	 * Devuelve los errores que se hayan producido
	 *
	 * @access public
	 * @return array
	 */
	public function getError(){
		if(empty($this->aError)) return false;
		return($this->aError);
	}

	/**
	 * Muestra el texto asignado a un error
	 * Si no se pasa ninguna clave se muestran todos.
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
	 * Devuelve el nombre del formulario (sin prefijo "frm")
	 *
	 * @since 2011-08-16
	 * @access public
	 * @return string
	 */
	public function getFormNombre(){
		return($this->sFormNombre);
	}

	/**
	 * Devuelve el codigo Javascript creado durante el procesado de los campos
	 *
	 * @since 2011-08-24
	 * @access public
	 * @return string
	 */
	public function getJavaScript(){
		$aRet = $this->aJavaScript;
		$ret = '';

		if(!empty($aRet)){
			$ret = '<script type="text/javaScript"><!--'."\n";

			if(!empty($aRet['depende'])){
				$aTemp = array();
				foreach($aRet['depende'] as $clave => $valor){
					$aTemp[] = '\''.$clave.'\':['.$valor.']';
				}
				$ret .= 'var depende_'.$this->sFormNombre.' = {'.implode(",", $aTemp).'};'."\n";
				unset($aRet['depende']);
			}
			if(!empty($aRet['ligado'])){
				$ret .= 'var ligado_'.$this->sFormNombre.' = {'.implode(",", $aRet['ligado']).'};'."\n";
				unset($aRet['ligado']);
			}

			$ret .= implode("\n", $aRet);
			$ret .= "\n//--></script>\n";
		}

		return($ret);
	}

	/**
	 * Muestra el texto asignado a un mensaje o fragmento HTML
	 * Si no se pasa ninguna clave se muestra la lista de textos visibles por los usuarios finales usados en funciones JavaScript y HTML
	 *
	 * @todo mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @access public
	 * @param $clave Clave del mensaje
	 * @return string
	 */
	public function getMetodosMensaje($clave = false){
		if($clave === false) return($this->aMetodosMensajes);
		return($this->aMetodosMensajes[$clave]);
	}

	/**
	 * Muestra los tiempos consumidos en la generacion del formulario, elementos, etc
	 *
	 * @since 20110720
	 * @access public
	 * @return array
	 */
	public function getTiempos(){
		return($this->aTiempos);
	}

	/**
	 * Devuelve microtime en segundos
	 *
	 * @access protected
	 * @param boolean $float Si true devuelve un numero flotante, si false devuelve formateado "legible para humanos" (parecido a ISO 8601)
	 * @return float
	 */
	protected function microtimeSeg($float = false){
		list($mseg, $seg) = explode(" ", microtime());

		if($float) return((float)$mseg + (float)$seg);
		else return(date('Y-m-d H:i:s', $seg).substr($mseg, 1));
	}

	/**
	 * Pinta el formulario. Recomendable sobreescribirlo en clases hijas destinadas a dar mejor aspecto al formulario
	 *
	 * @todo pintado de elementos agrupados (group) y su contenido (otros elementos)
	 * @access public
	 * @param array $campos Campos del formulario
	 * @param boolean $echo Devuelve el formulario (false) o lo escribe (true, por defecto)
	 * @return string
	 */
	public function pintaFormulario($campos = array(), $echo = true){
		if(count($campos) && !count($this->aFormulario)) $this->creaFormulario($campos);
		$sEcho = '';

		//medicion de tiempo
		$this->aTiempos['pintaFormulario'] = $this->microtimeSeg(true);

		$sEcho .= $this->aFormulario['frmInicio']['campo'].$this->aFormulario['frmInicio']['texto'];
		$i = 0;
		$aEcho = array();
		$sHidden = ''; //agrupa los campos ocultos al final, para evitar problemas de validacion y dibujado en la pagina
		foreach($this->aFormulario['frmElementos'] as $clave => $valor){
			if($this->aCampos[$clave]['tipo'] != 'hidden'){
				//clases css
				$css = array();
				if(!$i) $css[] = $this->aCss['titulo']; //primera fila
				if($i%2) $css[] = $this->aCss['fimpar']; //fila impar
				else $css[] = $this->aCss['fpar']; //fila par
				if($i == count($this->aFormulario['frmElementos'])-1) $css[] = $this->aCss['ultima']; //ultima fila
				if(!empty($css)) $css = ' class="'.implode(' ', $css).'"';

				//guardando en una variable para su posterior dibujado
				if($this->aCampos[$clave]['tipo'] != 'group'){
					$sApertura = (isset($valor['extra']['p0'])?$valor['extra']['p0']."\n":'').'<p id="'.$clave.$this->sFormNombre.'Fil"'.$css.((!empty($this->aCampos[$clave]['depende']) && (empty($this->aCampos[$clave]['depende']['hid']) || $this->aCampos[$clave]['depende']['hid'] != 'tag') && (isset($this->aCampos[$this->aCampos[$clave]['depende']['id']]['valor']) && $this->aCampos[$clave]['depende']['val'] != $this->aCampos[$this->aCampos[$clave]['depende']['id']]['valor']))?' style="display:none;"':'').'>'.(isset($valor['texto'])?$valor['texto']:'').$valor['campo'].(empty($valor['help'])?'':$valor['help']);
					$sCierre = '</p>'.(isset($valor['extra']['p1'])?$valor['extra']['p1']."\n":'');
				}
				else{
					$sApertura = (isset($valor['extra']['p0'])?$valor['extra']['p0']."\n":'').str_replace('<fieldset', '<fieldset'.((!empty($this->aCampos[$clave]['depende']) && (empty($this->aCampos[$clave]['depende']['hid']) || $this->aCampos[$clave]['depende']['hid'] != 'tag') && (isset($this->aCampos[$this->aCampos[$clave]['depende']['id']]['valor']) && $this->aCampos[$clave]['depende']['val'] != $this->aCampos[$this->aCampos[$clave]['depende']['id']]['valor']))?' style="display:none;" ':' '), $valor['texto']);
					$sCierre = $valor['help'].$valor['campo'].(isset($valor['extra']['p1'])?$valor['extra']['p1']."\n":'');
				}

				$i++;

				if(!empty($this->aCampos[$clave]['group'])){
					$aEcho[$this->aCampos[$clave]['group']]['g'][$clave] = array($sApertura, $sCierre);
				}
				else $aEcho[$clave] = array($sApertura, $sCierre);/**/
			}
			else $sHidden .= $valor['campo'];
		}

		$sEcho .= $this->agrupaElementos($aEcho).$this->aFormulario['frmFinal']['texto'].$sHidden.$this->aFormulario['frmFinal']['campo'];

		//medicion de tiempo
		$tempTime = $this->microtimeSeg(true);
		$this->aTiempos['pintaFormulario'] = $tempTime - $this->aTiempos['pintaFormulario'];
		$this->aTiempos['total'] += $this->aTiempos['pintaFormulario'];

		if($echo) echo($sEcho);
		else return($sEcho);
	}

	/**
	 * Asigna la accion del formulario
	 *
	 * @since 2011-10-19
	 * @access public
	 * @param string $sActionHide Accion del formulario
	 * @return boolean
	 */
	public function setActionHide($sActionHide = false){
		//if(empty($sActionHide)) return(false);
		switch(strtolower($sActionHide)){
			case 'ins':
			case 'upd':
			case 'del':
			case 'busc':
				$this->sActionHide = strtolower($sActionHide);
				break;
			default:
				//error
				$this->sActionHide = false;
				return(false);
		}
		return(true);
	}

	/**
	 * Recoge y prepara los campos que formaran el formulario
	 *
	 * @since 2011-07-20
	 * @access public
	 * @param array $campos Array (o texto XML) con los campos que formaran el formulario
	 * @return boolean
	 */
	public function setCampos($campos = array()){
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
				if($clave == 'id'){
					$this->sFormNombre = (string)$valor;
					if(strpos($this->sFormNombre, 'frm') === 0) $this->sFormNombre = substr($this->sFormNombre, 3); //el nombre automatico es un crc16, no empieza por 'frm'
				}
			}
			foreach($xmlArray->children() as $valor){ //convertir elementos de xml en elementos del formulario
				$this->xmlForm($valor);
			}

			foreach($this->aCampos as $clave => $valor){
				if(!isset($valor['tipo'])) unset($this->aCampos[$clave]); //evita procesar campos dedicados exclusivamente al pintado de listados
			}

			//medicion de tiempo
			$this->aTiempos['setCampos'] = $this->microtimeSeg(true) - $this->aTiempos['setCampos'];
		}
		elseif(count($campos)){ //origen array
			$this->aCampos = array();
			foreach($campos as $clave => $valor){
				if(isset($valor['tipo'])) $this->aCampos[$clave] = $valor; //evita procesar campos dedicados exclusivamente al pintado de listados
			}
		}
		else{ //vacio, origen desconocido o ya asignado en Formazo::setCampos()
			if(empty($this->aCampos)){
				$this->setError(__METHOD__.'_2');
				return(false);
			}
		}

		//recoge los valores enviados, si es el caso (formulario enviado a la misma pagina, por ejemplo)
		//recoge GET o POST segun el que se haya utilizado para llegar a esta pagina
		if($this->sMethod == 'get') $this->aVariablesRequest = &$_GET;
		elseif($this->sMethod == 'post') $this->aVariablesRequest = &$_POST;
		else $this->aVariablesRequest = array();
		//TODO opcion para recogerlos o no
		if(empty($this->aCampos)){
			//TODO error por formulario vacio
			return(false);
		}
		foreach($this->aCampos as $clave => $valor){
			if(isset($this->aVariablesRequest[$clave]) && $this->aVariablesRequest[$clave] != '') $this->aCampos[$clave]['valor'] = $this->aVariablesRequest[$clave];
			//NO USADO, INSEGURO
			//limpia el valor de dataSource y sustituye $id$ por la id del campo
			/*if(isset($this->aCampos[$clave]['dataSource'])){
				$aTemp = explode(':', $this->aCampos[$clave]['dataSource']);
				switch(strtolower($aTemp[0])){
					case 'js':
					case 'php':
						$aTemp[0] = strtolower($aTemp[0]);
						break;
					default:
						if(isset($aTemp[1])) $aTemp[0] = 'js'; //tipo de idioma de la llamada de origen de datos no reconocida, deberia devolverse error?
						else{
							$aTemp[1] = $aTemp[0];
							$aTemp[0] = 'js'; //por defecto el idioma usado es javascript
						}
				}
				$aTemp[1] = str_replace('$id$', $clave, $aTemp[1]); //$id$ debe ir encerrado entre comillas
				$this->aCampos[$clave]['dataSource'] = $aTemp;
			}*/
			//comprueba que dataSource sea una de las funciones permitidas
			/*if(isset($this->aCampos[$clave]['dataSource'])){
				@include_once(dirname(__FILE__).'/sourceCode.php');
				global $aSourceCodeFunctions;
				if(isset($this->aSourceFuncs) && !in_array($this->aCampos[$clave]['dataSource'], $this->aSourceFuncs)){
					unset($this->aCampos[$clave]['dataSource']);
					//error, funcion no permitida, puede ser intento maligno
				}
			}*/
		}

		return(true);
	}

	/**
	 * Asigna clases CSS
	 *
	 * @since 201-12-24
	 * @access public
	 * @param mixed $clave Clave del array CSS; array completo o false para valores por defecto
	 * @param mixed $clave Clase CSS para la clave
	 * @return boolean
	 */
	protected function setCss($clave = false, $clase = false){
		$aCss = array();

		if($clave === false){ //reinicio de clases CSS
			$aCss = array(
			 'namespace' => 'frm', //pseudo espacio de nombres para todas las clases, como prefijo; menos para "dis", que es sufijo de "disabled"
			 'dis' => 'Dis', //sufijo para elementos desabilitados ("disabled")
			//pintado de formularios
			 'fimpar' => 'FilaImpar', 'fpar' => 'FilaPar', 'titulo' => 'Titulo', 'ultima' => 'Ultima',
			//elementos del formulario
			 'button' => 'Button', 'checkbox' => 'Check', 'file' => 'File', 'label' => 'Label', 'multiple' => 'Select', 'password' => 'Passwd', 'radio' => 'Radio', 'select' => 'Select', 'text' => 'Text', 'textarea' => 'TArea',
			//elementos logicos o informativos
			 'group' => 'Group', 'heading' => 'Heading', 'help' => 'Help', 'hint' => 'Hint', 'repeat' => 'Repeat',
			//validaciones
			 'fecha' => 'Fecha', 'obligatorio' => 'Oblig'
			);
		}
		else{
			if(is_array($clave)) $aCss = $clave;
			else $aCss[$clave] = $valor;

		}

		$sNamespace = isset($aCss['namespace'])?$aCss['namespace']:(isset($this->aCss['namespace'])?$this->aCss['namespace']:'');

		foreach($aCss as $c => $v){
			$this->aCss[$c] = (($c != 'namespace' && $c != 'dis')?$sNamespace:'').$v;
		}
		return(true);
	}

	/**
	 * Guarda los errores que se hayan producido
	 *
	 * @access protected
	 * @return string
	 */
	protected function setError(){
		$argumentos = func_get_args();
		if(empty($argumentos)) return(false);

		$clave = array_shift($argumentos);

		$this->aError[] = $clave.' ['.$this->microtimeSeg().'] '.vsprintf($this->aErrorMensajes[$clave], $argumentos);
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
			//TODO definir constantes de tipos de error para usar trigger_error() ademas de esto
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aErrorMensajes = array(
			'Formazo::creaFormulario' => 'Tipo de elemento ("%s") no reconocido.',
			'Formazo::creaFormulario_2' => 'No se han podido procesar los campos.',
			'Formazo::setCampos' => 'Error XML: %s',
			'Formazo::setCampos_2' => 'No se puede crear un formulario sin campos.',
			'FormazoElementos::elemento' => 'Tipo de validación (%s) no reconocido.',
			'FormazoPassword::elemento' => 'Tipo de validación (%s) no reconocido.',
			'FormazoText::elemento' => 'Tipo de validación (%s) no reconocido.',
			'FormazoTextarea::elemento' => 'Tipo de validación (%s) no reconocido.'
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
	 * Asigna el nombre del formulario (sin "frm")
	 *
	 * @since 2011-08-16
	 * @access public
	 * @param string $formName Parte variable del nombre de formulario
	 * @return string
	 */
	public function setFormNombre($formName = ''){
		//nombre del formulario (si no se ha asignado valor a esta propiedad)
		//mt_srand(crc32(serialize($this->aCampos)) + crc32(microtime())); //iniciar semilla aleatoria; necesario para los nombres de formulario por defecto
		//$this->sFormNombre = mt_rand();

		if($formName != '') $this->sFormNombre = $formName;
		elseif($this->sFormNombre == '' && !empty($this->aCampos)) $this->sFormNombre = $this->crc16(serialize($this->aCampos));

		return(true);
	}

	/**
	 * Asigna el fichero de imagen que se usara como icono de "hint" (texto de ayuda de los campos)
	 *
	 * @since 2012-03-28
	 * @access public
	 * @param string $hint Fichero de imagen (con ruta)
	 * @return boolean
	 */
	public function setHintIco($hint = '#'){
		if(empty($hint)) return(false);
		$this->sHintIco = $hint;
		return(true);
	}

	/**
	 * Cambia los actuales textos visibles por los usuarios finales usados en funciones javascript y html
	 * Util, por ejemplo, para traducciones.
	 * Si clave igual a false se reinicializan los mensajes.
	 *
	 * @access public
	 * @param mixed $clave Clave del mensaje, o un array con todos los mensajes {array('clave' => 'mensaje',[...]);}
	 * @param string $valor Nuevo texto para el mensaje
	 * @return boolean
	 */
	public function setMetodosMensaje($clave = false, $valor = false){
		if($clave === false){ //reinicio de mensajes
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aMetodosMensajes = array(
			'Formazo::creaFormulario_1' => 'Esta accion no podrá deshacerse.\\n¿Desea continuar?',
			'Formazo::creaFormulario_2' => 'Esta accion no podrá deshacerse.\\n¿Desea continuar?',
			'Formazo::creaFormulario_3' => 'Limpiar valores',
			'Formazo::creaFormulario_4' => 'Cancelar',
			'Formazo::creaFormulario_5' => 'Si ha realizado cambios estos se perder\u00e1n.',
			'Formazo::creaFormulario_6' => 'Si ha realizado cambios estos se perder\u00e1n.',
			'FormazoFile::elemento_1' => 'Tama&ntilde;o m&aacute;ximo (bytes): ',
			'FormazoFile::elemento_2' => 'Fichero actual: ',
			'FormazoFile::elemento_3' => 'Borra fichero',
			'FormazoFile::elemento_4' => 'Actualice para completar.',
			'FormazoPassword::elemento' => 'Confirme '
			);
			return(true);
		}

		$aMensajes = array();

		if(!is_array($clave)) $aMensajes[$clave] = $valor;
		else $aMensajes = $clave;

		foreach($aMensajes as $c => $v){
			$this->aMetodosMensajes[$c] = $v;
		}
		return(true);
	}

	/**
	 * Asigna la ruta donde subir ficheros
	 *
	 * @todo comprobar si la ruta proporcionada es escribible, y accion en caso contrario
	 * @since 2011-10-19
	 * @access public
	 * @param string $sRutaFicheros Ruta donde subir ficheros
	 * @return boolean
	 */
	public function setRutaFicheros($sRutaFicheros = false){
		if(!empty($sRutaFicheros)) $this->sRutaFicheros = $sRutaFicheros;
		return(true);
	}

	/**
	 * Asigna la lista de funciones que se permitiran para rellenar selects que tengan 'dataSource'
	 * $funcs sera un array con las funciones permitidas, 'Etiqueta visible para el usuario' => 'nombre_de_la_funcion' o solo 'nombre_de_la_funcion' (que sera tambien usada como etiqueta)
	 *
	 * @access public
	 * @param array $funcs Lista de funciones permitidas
	 * @return boolean
	 */
	public function setSourceFuncs($funcs = false){
		if(empty($funcs)) return(false);
		if(!is_array($funcs)){
			$funcs = array($funcs);
		}

		$this->aSourceFuncs = array();
		foreach($funcs as $clave => $valor){
			if(is_callable($valor)) $this->aSourceFuncs[$clave] = $valor;
		}

		return(true);
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
			if(!isset($mode)) $mode = 'repeat';
		}
		elseif(isset($tipo) && strtolower($tipo) == 'comment'){
			$tipo = 'heading';
			$mode = 'comment';
		}

		$this->aCampos[$id]['tipo'] = $tipo; //TODO revisar grupos (por ejemplo para group/block/tab)
		if($tipo == 'select' && isset($mode) && $mode == 'checkbox') $this->aCampos[$id]['tipo'] = 'multiple'; //convierte en select multiple aquellos que han de dibujarse con checkbox

		if(!empty($validation)) $this->aCampos[$id]['validacion'] = $validation=='date'?'fecha':$validation;
		if(!empty($required)) $this->aCampos[$id]['obligatorio'] = $required;
		if(!empty($length)) $this->aCampos[$id]['longitud'] = $length;
		/*if(!empty($depende)) $this->aCampos[$id]['depende']['id'] = $depende;
		if(!empty($depende_val)) $this->aCampos[$id]['depende']['val'] = $depende_val;*/
		if(!empty($ligado)) $this->aCampos[$id]['ligado'] = $ligado;
		if(!empty($separador)) $this->aCampos[$id]['separador'] = $separador;
		if(!empty($css)) $this->aCampos[$id]['css'] = $css;
		if(!empty($mode)) $this->aCampos[$id]['extra']['modo'] = $mode;
		if(!empty($isHTML)) $this->aCampos[$id]['isHTML'] = $isHTML;

		if(!empty($group)) $this->aCampos[$id]['group'] = $group;

		//resto de propiedades y otros elementos contenidos
		foreach($xml->children() as $clave => $valor){
			switch($clave){
				case 'choices':
					$tempChoices = $valor->children();
					foreach($tempChoices->item as $claveCh => $valorCh){
						$this->aCampos[$id]['valores'][(string)$tempChoices->label][(string)$valorCh->value] = (string)$valorCh->label;
					}
					break;
				case 'dataSource': //carga dinamica de selects
					$this->aCampos[$id]['dataSource'] = (string)$valor; //se comprueba y añade la id del campo como parametro cuando se use
					break;
				case 'depends': //campos/zonas dependientes
					$this->aCampos[$id]['depende']['id'] = (string)$valor->label; //id de la zona de la que dependen
					$this->aCampos[$id]['depende']['val'] = (string)$valor->value; //valor que debe tener la zona de la que depende para que se muestre este
					break;
				case 'format': //TODO formato de fechas
					$this->aCampos[$id]['extra']['modo'] = (string)$valor;
					break;
				case 'help';
				case 'hint';
					$this->aCampos[$id][$clave] = (string)$valor;
					break;
				case 'item':
					$this->aCampos[$id]['valores'][(string)$valor->value] = (string)$valor->label;
					break;
				case 'label':
					$this->aCampos[$id]['titulo'] = (string)$valor;
					break;
				case 'value':
					if(isset($this->aCampos[$id]['valor'])){ //permite que un solo campo tenga mas de un valor por defecto; necesario para "repeat", en cuyo caso se construyen varios elementos repetidos iniciales
						if(!is_array($this->aCampos[$id]['valor'])) $this->aCampos[$id]['valor'] = array($this->aCampos[$id]['valor']);
						$this->aCampos[$id]['valor'][] = (string)$valor;
					}
					else $this->aCampos[$id]['valor'] = (string)$valor;
					break;
				default:
					//if(!isset($valor->attributes()->id)){var_dump($valor);die();}
					$this->xmlForm($valor, ($tipo=='group'?$id:false));
					//if($valor->getName());
			}
		}

		return(true);
	}

}

//* * * * * * * * * * * * * * * * * * *
// Clases para elementos de formulario *
//* * * * * * * * * * * * * * * * * * *

/**
 * Clase abstracta de clases para elementos del formulario
 *
 * @todo un valor mas que guarde el valor actual del campo?
 * @since 2011-06-22
 */
abstract class FormazoElementos {
	protected $sClave; //Nombre del campo

	/**
	 * Caracteristicas de los elementos; de cual depende, titulo, valor, etc
	 *
	 * @var array
	 * @see FormazoElementos::setDatos($datos)
	 * @access private
	 */
	protected $aDatos;

	protected $sActionHide; //Accion del formulario
	protected $aCss; //Clases CSS que daran estilo al elemento
	protected $aExtra; //Elementos extra asociados
	protected $sFormNombre; //Nombre del formulario

	/**
	 * Lista de funciones permitidas para rellenar selects
	 *
	 * @var array
	 * @see FormazoElementos::setSourceFuncs($funcs)
	 * @access protected
	 */
	protected $aSourceFuncs;

	protected $iTabIndex; //Indice de tabulacion
	protected $sRutaFicheros; //Ruta donde subir ficheros

	/**
	 * Lista de textos visibles por los usuarios finales usados en html
	 *
	 * @var array
	 * @see FormazoElementos::setMetodosMensajes($clave, $valor),FormazoElementos::getMetodosMensajes($clave)
	 * @access private
	 */
	private $aMetodosMensajes;

	/**
	 * Mensajes de error
	 *
	 * @var string
	 * @see FormazoElementos::getError(),FormazoElementos::setError()
	 * @access public
	 */
	protected $aError;

	/**
	 * Icono de los mensajes de ayuda de los campos
	 *
	 * @var string
	 * @see FormazoElementos::setHintIco()
	 * @access protected
	 */
	protected $sHintIco;

	/**
	 * Mensajes de error la clase, a fin de que puedan ser traducidos/modificados
	 * Las secuencias %s deben dejarse ya que corresponden a variables que seran luego sustituidas, si es necesario poner un % se pondra como %%
	 *
	 * @var array
	 * @see FormazoElementos::setErrorMensaje($clave, $valor),FormazoElementos::getErrorMensaje($clave)
	 * @access public
	 */
	protected $aErrorMensajes;

	function __construct($clave, $valor){
		$this->sClave = $clave;
		//$aTemp = explode(' ', $this->aFormulario['frmElementos'][$clave]['propiedades']); //0 tipo, 1 validacion, 2 obligatorio, 3 longitud maxima (=0 -> disabled)
		$this->setDatos($valor);

		$this->aError = array();
		$this->setErrorMensaje();
		$this->setMetodosMensaje(); //inicializacion de los textos visibles de la clase

		$this->sActionHide = 'ins';
		$this->aCss = array();
		$this->aExtra = array();
		$this->sFormNombre = '';
		$this->iTabIndex = 0;
		$this->sHintIco = '#';

		//TODO evitar problemas con codigo html en tooltips
		//TODO codificacion
	}

	/**
	 * Elemento del formulario
	 *
	 * @since 2011-06-24
	 * @access protected
	 * @abstract
	 * @return array
	 */
	abstract protected function elemento();

	/**
	 * Devuelve el elemento formado, validaciones, etc
	 *
	 * @since 2011-06-22
	 * @access public
	 * @return array
	 */
	public function getElemento(){
		return($this->elemento());
	}

	/**
	 * Devuelve los errores que se hayan producido
	 *
	 * @access public
	 * @return array
	 */
	public function getError(){
		if(empty($this->aError)) return false;
		return($this->aError);
	}

	/**
	 * Muestra el texto asignado a un error
	 * Si no se pasa ninguna clave se muestran todos.
	 *
	 * @todo mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @access public
	 * @param string $clave Clave del mensaje de error, si se omite (o false) muestra el array completo
	 * @return string
	 */
	public function getErrorMensaje($clave = false){
		if($clave === false) return($this->aErrorMensajes);
		return($this->aErrorMensajes[$clave]);
	}

	/**
	 * Muestra el texto asignado a un fragmento HTML
	 * Si no se pasa ninguna clave se muestra la lista de textos visibles por los usuarios finales usados en funciones JavaScript y HTML
	 *
	 * @todo mostrar la clave de un texto de mensaje de error (invirtiendo el array y un foreach para buscar coincidencia de texto)
	 * @access public
	 * @param string $clave Clave del mensaje
	 * @return string
	 */
	public function getMetodosMensaje($clave = false){
		if($clave === false) return($this->aMetodosMensajes);
		return($this->aMetodosMensajes[$clave]);
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
	 * Recoge la accion del formulario
	 *
	 * @since 2011-08-25
	 * @access public
	 * @param string $sActionHide Accion del formulario
	 * @return boolean
	 */
	public function setActionHide($sActionHide = false){
		if(empty($sActionHide)) return(false);
		$this->sActionHide = $sActionHide;
		return(true);
	}

	/**
	 * Recoge clases CSS
	 *
	 * @since 2011-06-24
	 * @access public
	 * @param array $aCss Clases CSS a aplicar
	 * @return boolean
	 */
	public function setCss($aCss = array()){
		if(empty($aCss)) return(false);
		$this->aCss = $aCss;
		return(true);
	}

	protected function setDatos($datos){
		$this->aDatos = empty($datos)?array():$datos;

		$campos = array('help', 'hint', 'longitud', 'titulo'); //lista de campos a comprobar

		foreach($campos as $valor){
			if($valor == 'hint'){
				//$this->aDatos[$valor] = isset($this->aDatos[$valor])?' title="'.htmlentities($this->aDatos[$valor]).'"':'';
				//$this->aDatos[$valor] = isset($this->aDatos[$valor])?'&nbsp;<img src="#" class="aCssTags" alt="'.htmlentities($this->aDatos[$valor]).'" title="'.htmlentities($this->aDatos[$valor]).'" />':'';
				$this->aDatos[$valor] = isset($this->aDatos[$valor])?'&nbsp;<img src="#" class="aCssTags" alt="'.$this->aDatos[$valor].'" title="'.$this->aDatos[$valor].'" />':'';
			}
			elseif($valor == 'help'){
				//$this->aDatos[$valor] = empty($this->aDatos[$valor])?'':'<span'.(empty($this->aCss[$valor])?'':' class="'.$this->aCss[$valor].'"').'>'.$this->aDatos[$valor].'</span>';
				$this->aDatos[$valor] = empty($this->aDatos[$valor])?'':'<span'.(empty($this->aCss[$valor])?'':' class="aCssTags"').'>'.$this->aDatos[$valor].'</span>';
			}
			else{
				$this->aDatos[$valor] = isset($this->aDatos[$valor])?$this->aDatos[$valor]:'';
			}
		}
		if(!isset($this->aDatos['separador'])) $this->aDatos['separador'] = ',';

		return(true);
	}

	/**
	 * Guarda los errores que se hayan producido
	 *
	 * @access protected
	 * @return string
	 */
	protected function setError(){
		$argumentos = func_get_args();

		if(empty($argumentos)) return(false);

		$clave = array_shift($argumentos);

		$this->aError[] = vsprintf($this->aErrorMensajes[$clave], $argumentos);
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
			//TODO definir constantes de tipos de error para usar trigger_error() ademas de esto
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aErrorMensajes = array();
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
	 * Recoge los extras del elemento
	 *
	 * @since 2011-06-24
	 * @access public
	 * @param array $aExtra Extras de los campos
	 * @return boolean
	 */
	public function setExtra($aExtra = array()){
		if(empty($aExtra)) return(false);
		$this->aExtra = $aExtra;
		return(true);
	}

	/**
	 * Recoge el nombre del formulario
	 *
	 * @since 2011-06-24
	 * @access public
	 * @param string $sFormNombre Nombre del formulario
	 * @return boolean
	 */
	public function setFormNombre($sFormNombre = ''){
		if(empty($sFormNombre)) return(false);
		$this->sFormNombre = $sFormNombre;
		return(true);
	}

	/**
	 * Asigna el fichero de imagen que se usara como icono de "hint" (texto de ayuda de los campos)
	 *
	 * @since 2012-03-28
	 * @access public
	 * @param string $hint Fichero de imagen (con ruta)
	 * @return boolean
	 */
	public function setHintIco($hint = '#'){
		if(empty($hint)) return(false);
		$this->sHintIco = $hint;
		return(true);
	}

	/**
	 * Actuales textos visibles por los usuarios finales usados en funciones JavaScript y HTML
	 * Util, por ejemplo, para traducciones
	 * Si clave igual a false se reinicializan los mensajes
	 *
	 * @access public
	 * @param mixed $clave Clave del mensaje, o un array con todos los mensajes {array('clave' => 'mensaje',[...]);}
	 * @param string $valor Nuevo texto para el mensaje
	 * @return boolean
	 */
	public function setMetodosMensaje($clave = false, $valor = false){
		if($clave === false){ //reinicio de mensajes
			//TODO se pierden los mensajes iniciales de las clases herederas
			$this->aMetodosMensajes = array();
			return(true);
		}

		$aMensajes = array();

		if(!is_array($clave)) $aMensajes[$clave] = $valor;
		else $aMensajes = $clave;

		foreach($aMensajes as $c => $v){
			$this->aMetodosMensajes[$c] = $v;
		}
		return(true);
	}

	/**
	 * Asigna la ruta de subida de ficheros
	 *
	 * @since 2011-10-19
	 * @access public
	 * @param string $sRutaFicheros Ruta de subida de ficheros, por defecto el directorio actual
	 * @return boolean
	 */
	public function setRutaFicheros($sRutaFicheros = ''){
		$this->sRutaFicheros = empty($sRutaFicheros)?'./':rtrim($sRutaFicheros, '\\/').'/'; //pone el ultimo /
		return(true);
	}

	/**
	 * Asigna la lista de funciones que se permitiran para rellenar selects que tengan 'dataSource'
	 *
	 * @access protected
	 * @param array $funcs Lista de funciones permitidas
	 * @return boolean
	 */
	public function setSourceFuncs($funcs = false){
		if(empty($funcs)) return(false);
		$this->aSourceFuncs = $funcs;

		return(true);
	}

	/**
	 * Recoge el indice de tabulacion
	 *
	 * @since 2011-06-24
	 * @access public
	 * @param integer $iTabIndex Indice de tabulacion
	 * @return boolean
	 */
	public function setTabIndex($iTabIndex = 0){
		if(empty($iTabIndex)) return(false);
		$this->iTabIndex = $iTabIndex;
		return(true);
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
		$aElemento = $aCssTag = array();
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2].'<input id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' type="checkbox" tabindex="'.$this->iTabIndex.'"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'])?' checked="checked"':'').((!empty($sSufijNombreCampo))?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' />'.$this->aExtra[3];

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':str_replace('aCssTags', (empty($this->aCss['help'])?'':$this->aCss['help']), $aElemento['help']);

		if(!empty($sSufijNombreCampo)) $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'" />';

		return($aElemento);
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
		$aElemento = $aCssTag = array();
		$iMaxUpload = 0;
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];

		//fijar tamano maximo
		//tamano maximo del fichero a subir en bytes (es facil saltarse esto, recontrolar con php en procesado del formulario)
		$iMaxUpload = min($this->let_to_num(ini_get('post_max_size')), $this->let_to_num(ini_get('upload_max_filesize')));
		if(isset($this->aDatos['longitud']) && $this->aDatos['longitud'] > 0){
			$iMaxUpload = min($iMaxUpload, $this->aDatos['longitud']);
		}
		$this->aFormulario['frmElementos'][$this->sClave]['campo'] = '<input type="hidden" name="MAX_FILE_SIZE" value="'.$iMaxUpload.'" />';
		$aElemento['campo'] = $this->aExtra[2].'<input id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.'"'.$sCssTag.' type="'.$this->aDatos['tipo'].'" tabindex="'.$this->iTabIndex.'"'.(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' />'.$this->aExtra[3];
		$aElemento['campo'] .= '<br />'.$this->getMetodosMensaje(__METHOD__.'_1').$iMaxUpload.' bytes';

		$aElemento['campo'] .= '<input id="hid_'.$this->sClave.$this->sFormNombre.'" type="hidden" name="hid_'.$this->sClave.'" value="'.(isset($this->aDatos['valor'])?$this->aDatos['valor']:'').'" />'; //ademas del fichero, se envia el valor del fichero actual en un "hidden", con el mismo nombre que el "input" del fichero y prefijo "hid_"; para actualizaciones (por ejemplo)
		$aElemento['campo'] .= '<input id="hid_'.$this->sClave.$this->sFormNombre.'d" type="hidden" name="hid_'.$this->sClave.'d" value="" />'; //este sirve para borrar el fichero sin tocar el registro //TODO cambiar (variable sesion?), inseguro

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		//muestra la imagen (si lo es), o el nombre del fichero actual
		if(isset($this->aDatos['valor']) && $this->aDatos['valor'] != ''){
			$aElemento['file'] = '<br /><div id="div_'.$this->sClave.$this->sFormNombre.'">';
			switch(strtolower(substr($this->aDatos['valor'], strrpos($this->aDatos['valor'], '.')+1))){
				case 'jpg':
				case 'jpeg':
				case 'gif':
				case 'png':
					$aElemento['file'] .= '<img'.$sCssTag.' src="'.$this->sRutaFicheros.rawurldecode($this->aDatos['valor']).'" alt="Imagen actual" title="Imagen actual" />';
					break;
				default:
					$aElemento['file'] .= $this->getMetodosMensaje(__METHOD__.'_2').'<a href="'.$this->sRutaFicheros.rawurldecode($this->aDatos['valor']).'" target="_blank">'.$this->aDatos['valor'].'</a>';
			}
			//boton de borrado de ficheros
			$aElemento['file'] .= '<br /><input id="but_'.$this->sClave.$this->sFormNombre.'" class="but_del_file" type="button" tabindex="'.$this->iTabIndex.'" value="'.$this->getMetodosMensaje(__METHOD__.'_3').'" />';
			//$aElemento['validacionIL'] = '$("#but_'.$this->sClave.$this->sFormNombre.'").addEvent("click", function (){document.getElementById(\'hid_'.$this->sClave.$this->sFormNombre.'d\').value = \'del\';document.getElementById(\'div_'.$this->sClave.$this->sFormNombre.'\').innerHTML = \''.$this->getMetodosMensaje(__METHOD__.'_4').'\';});';
			$aElemento['file'] .= '</div>';
		}

		return($aElemento);
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

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = '';
		if(isset($this->aDatos['extra']['modo'])){
			if(strtolower($this->aDatos['extra']['modo']) == 'list' || strtolower($this->aDatos['extra']['modo']) == 'repeat' || strtolower($this->aDatos['extra']['modo']) == 'row') $sCssLabel .= $this->aCss['repeat'].' ';
			else $sCssLabel = $this->aDatos['extra']['modo'].' ';
		}
		if(!empty($this->aCss['group'])) $sCssLabel = ' class="'.$sCssLabel.$this->aCss['group'].'"';
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];

		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<fieldset id="'.$this->sClave.$this->sFormNombre.'"'.$sCssLabel.'><legend>'.$this->aDatos['titulo'].'</legend>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2].'</fieldset>';

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		return($aElemento);
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
		$aElemento['campo'] = '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="'.$this->aDatos['tipo'].'" value="'.(isset($this->aDatos['valor'])?$this->aDatos['valor']:'').'" />';

		return($aElemento);
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
		$aElemento = $aCssTag = array();
		$this->aDatos['extra']['modo'] = (!empty($this->aDatos['extra']['modo']) && $this->aDatos['extra']['modo'] == 'checkbox')?'checkbox':'multiple'; //modo de dibujado del select [multiple|checkbox]
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2];
		if($this->aDatos['extra']['modo'] == 'multiple') $aElemento['campo'] .= '<select id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" multiple="multiple"'.(!empty($this->aDatos['longitud'])?' size="'.$this->aDatos['longitud'].'"':'').' name="'.$this->sClave.$sSufijNombreCampo.'[]"'.$sCssTag.' tabindex="'.$this->iTabIndex.'"'.(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').'>';
		elseif($this->aDatos['extra']['modo'] == 'checkbox') $aElemento['campo'] .= '<span id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" multiple="multiple"'.(!empty($this->aDatos['longitud'])?' size="'.$this->aDatos['longitud'].'"':'').' name="'.$this->sClave.$sSufijNombreCampo.'[]"'.$sCssTag.' tabindex="'.$this->iTabIndex.'"'.(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').'>';

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		//options
		if(isset($this->aSourceFuncs) && isset($this->aDatos['dataSource']) && in_array($this->aDatos['dataSource'], $this->aSourceFuncs)){
			$aElemento['campo'] .= $this->aDatos['dataSource'](empty($this->aDatos['valor'])?false:$this->aDatos['valor']); //TODO pasar tambien separador, $this->aDatos['separador']
		}
		elseif(isset($this->aDatos['valores']) && count($this->aDatos['valores']) > 0){
			$aTempVal = array();
			if(isset($this->aDatos['valor']) && strpos($this->aDatos['valor'], $this->aDatos['separador']) !== false){ //valores seleccionados
				$aTempVal = explode($this->aDatos['separador'], $this->aDatos['valor']);
				$aTempVal = array_map('trim', $aTempVal);
				$aTempVal = array_flip($aTempVal); //TODO posibilidad de que algun valor no se pueda usar como clave (por convenciones de nombrado)
			}
			elseif(isset($this->aDatos['valor']) && strpos($this->aDatos['valor'], $this->aDatos['separador']) === false) $aTempVal[trim($this->aDatos['valor'])] = 0;

			if(count($this->aDatos['valores']) > 0){
				$iCheckCont = 0;
				foreach($this->aDatos['valores'] as $c => $v){
					if($this->aDatos['extra']['modo'] == 'multiple'){
						if(is_array($v)){
							$aElemento['campo'] .= '<optgroup label="'.$c.'">';
							foreach($v as $c2 => $v2)
								$aElemento['campo'] .= '<option value="'.$c2.'"'.(isset($aTempVal[$c2])?' selected="selected"':'').'>'.$v2.'</option>';
							$aElemento['campo'] .= '</optgroup>';
						}
						else $aElemento['campo'] .= '<option value="'.$c.'"'.(isset($aTempVal[$c])?' selected="selected"':'').'>'.$v.'</option>';
					}
					elseif($this->aDatos['extra']['modo'] == 'checkbox'){
						if(is_array($v)){
							$aElemento['campo'] .= '<span>'.$c;
							foreach($v as $c2 => $v2)
								$aElemento['campo'] .= '<input id="c_'.$iCheckCont.$this->sClave.'" name="'.$this->sClave.$sSufijNombreCampo.'[]" type="checkbox" value="'.$c2.'"'.(isset($aTempVal[$c2])?' checked="checked"':'').' />'.'<label for="c_'.($iCheckCont++).$this->sClave.'"'.$sCssLabel.'>'.$v2.'</label>';
							$aElemento['campo'] .= '</span>';
						}
						else $aElemento['campo'] .= '<input id="c_'.$iCheckCont.$this->sClave.'" name="'.$this->sClave.$sSufijNombreCampo.'[]" type="checkbox" value="'.$c.'"'.(isset($aTempVal[$c])?' checked="checked"':'').' />'.'<label for="c_'.($iCheckCont++).$this->sClave.'"'.$sCssLabel.'>'.$v.'</label>';
					}
				}
			}
		}

		if($this->aDatos['extra']['modo'] == 'multiple') $aElemento['campo'] .= '</select>';
		if($this->aDatos['extra']['modo'] == 'checkbox') $aElemento['campo'] .= '</span>';
		$aElemento['campo'] .= $this->aExtra[3];
		if(!empty($sSufijNombreCampo)) $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'" />';

		return($aElemento);
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
		$aElemento = $aCssTag = array();
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2].'<input id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' type="'.$this->aDatos['tipo'].'" tabindex="'.$this->iTabIndex.'" value="'.(isset($this->aDatos['valor'])?$this->aDatos['valor']:'').'"'.(!empty($this->aDatos['longitud'])?' maxlength="'.$this->aDatos['longitud'].'"':'').(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' autocomplete="off" />'.$this->aExtra[3]; //autocomplete="off" es importante para que firefox no rellene con autocompletado (aunque solo funciona en carga de pagina, no cuando se tabula por los campos)
		if($sSufijNombreCampo == 'Dis') $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?$this->aDatos['valor']:'').'"'.((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' />';

		//confirmacion de password cuando no estamos en borrado
		if($this->sActionHide != 'del' && isset($this->aDatos['validacion']) && empty($sSufijNombreCamp)){
			$aElemento[2]['texto'] = '<label for="'.$this->sClave.$this->sFormNombre.'2"'.$sCssLabel.'>'.$this->getMetodosMensaje(__METHOD__).$this->aDatos['titulo'].'</label>';
			$aElemento[2]['campo'] = '<input id="'.$this->sClave.$this->sFormNombre.'2" name="'.$this->sClave.'2"'.$sCssTag.' type="'.$this->aDatos['tipo'].'" tabindex="'.$this->iTabIndex.'" value="'.(isset($this->aDatos['valor'])?$this->aDatos['valor']:'').'" autocomplete="off" />';
		}

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		return($aElemento);
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
		$aElemento = $aCssTag = array();
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<span'.$sCssLabel.'>'.$this->aDatos['titulo'].'</span>'.$this->aExtra[1];
		$aElemento['campo'] = '';

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		//botones
		$sJs = '';
		if(count($this->aDatos['valores']) > 0){
			$aElemento['campo'] .= $this->aExtra[2];
			foreach($this->aDatos['valores'] as $c => $v){
				$aElemento['campo'] .= '<label for="r_'.$c.$this->sFormNombre.'"'.$sCssLabel.'>'.$v.'</label>&nbsp;<input id="r_'.$c.$this->sFormNombre.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' type="radio" tabindex="'.$this->iTabIndex.'" value="'.$c.'"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c)?' checked="checked"':'').(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' />';
			}
			$aElemento['campo'] .= $this->aExtra[3];
		}

		if(!empty($sSufijNombreCampo)) $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'" />';

		return($aElemento);
	}
}

/**
 * Campo 'select'
 * ej. valores con grupos: 'valores' => array('grupo 1' => array('valor 1' => 'texto 1', 'valor 2' => 'texto 2', 'valor 3' => 'texto 3'), 'grupo 2' => array('valor 4' => 'texto 4', 'valor 5' => 'texto 5')).
 *
 * @since 2011-06-22
 */
class FormazoSelect extends FormazoElementos {
	function __construct($clave, $valor){
		parent::__construct($clave, $valor); //constructor de la clase padre
	}

	protected function elemento(){
		/*
		NO USADO, INSEGURO
		permitir al usuario poner una funcion para rellenar el select
		*/
		/*switch($this->aDatos['dataSource'][0]){
			case 'js':
				//se incluye la llamada a la pila de javascript, mas abajo
				break;
			case 'php':
				ob_start();
				$aElemento = eval('return '.$this->aDatos['dataSource'][1].';'); //asegura el punto y coma final, sin el dara error, o no continuara la ejecucion
				$eval_buffer = ob_get_contents();
				ob_end_clean();

				return($aElemento);
				break;
			default:
				//error, el lenguaje de programacion deberia ser uno de los anteriores
				//eso debe estar controlado en Formazo::setDatos()
		}*/

		$aElemento = $aCssTag = array();
		$this->aDatos['extra']['modo'] = (!empty($this->aDatos['extra']['modo']) && $this->aDatos['extra']['modo'] == 'radio')?'radio':'select'; //modo de dibujado del select [select|radio]
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2];
		if($this->aDatos['extra']['modo'] == 'select') $aElemento['campo'] .= '<select id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' tabindex="'.$this->iTabIndex.'"'.(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').'>';
		elseif($this->aDatos['extra']['modo'] == 'radio') $aElemento['campo'] .= '<span id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' tabindex="'.$this->iTabIndex.'"'.(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').'>';

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		//options
		if(isset($this->aSourceFuncs) && isset($this->aDatos['dataSource']) && in_array($this->aDatos['dataSource'], $this->aSourceFuncs)){
			$aElemento['campo'] .= $this->aDatos['dataSource'](empty($this->aDatos['valor'])?false:$this->aDatos['valor']);
		}
		elseif(isset($this->aDatos['valores']) && count($this->aDatos['valores']) > 0){
			$iRadioCont = 0;
			foreach($this->aDatos['valores'] as $c => $v){
				if($this->aDatos['extra']['modo'] == 'select'){
					if(is_array($v)){
						$aElemento['campo'] .= '<optgroup label="'.$c.'">';
						foreach($v as $c2 => $v2) $aElemento['campo'] .= '<option value="'.$c2.'"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c2)?' selected="selected"':'').'>'.$v2.'</option>';
						$aElemento['campo'] .= '</optgroup>';
					}
					else $aElemento['campo'] .= '<option value="'.$c.'"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c)?' selected="selected"':'').'>'.$v.'</option>';
				}
				elseif($this->aDatos['extra']['modo'] == 'radio'){
					if(is_array($v)){
						$aElemento['campo'] .= '<span>'.$c;
						foreach($v as $c2 => $v2) $aElemento['campo'] .= '<input id="r_'.$iRadioCont.$this->sClave.'" name="'.$this->sClave.$sSufijNombreCampo.'" type="radio" value="'.$c2.'"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c2)?' checked="checked"':'').' />'.'<label for="r_'.($iRadioCont++).$this->sClave.'"'.$sCssLabel.'>'.$v2.'</label>';
						$aElemento['campo'] .= '</span>';
					}
					else $aElemento['campo'] .= '<input id="r_'.$iRadioCont.$this->sClave.'" name="'.$this->sClave.$sSufijNombreCampo.'" type="radio" value="'.$c.'"'.((isset($this->aDatos['valor']) && $this->aDatos['valor'] == $c)?' checked="checked"':'').' />'.'<label for="r_'.($iRadioCont++).$this->sClave.'"'.$sCssLabel.'>'.$v.'</label>';
				}
			}
		}

		if($this->aDatos['extra']['modo'] == 'select') $aElemento['campo'] .= '</select>';
		elseif($this->aDatos['extra']['modo'] == 'radio') $aElemento['campo'] .= '</span>';
		$aElemento['campo'] .= $this->aExtra[3];
		if(!empty($sSufijNombreCampo)) $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'" />';

		return($aElemento);
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
		$aElemento = $aCssTag = array();
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion']) && $this->aDatos['validacion'] == 'fecha'){
			$aCssTag[] = $this->aCss['fecha']; //TODO util para colocar un "date picker" automaticamente, llevar al resto de elementos?
			//formato de fechas
			if(!empty($this->aDatos['obligatorio']) && isset($this->aDatos['valor']) && $this->aDatos['valor'] != '') $this->aDatos['valor'] = strftime('%Y/%m/%d %H:%M:%S', strtotime($this->aDatos['valor']));
		}
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2].'<input id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' type="'.$this->aDatos['tipo'].'" tabindex="'.$this->iTabIndex.'" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'"'.(!empty($this->aDatos['longitud'])?' maxlength="'.$this->aDatos['longitud'].'"':'').(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' />'.$this->aExtra[3];

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		if(!empty($sSufijNombreCampo)) $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'" />';
		//$this->aFormulario[$i] = $this->aFormulario['frmElementos'][$this->sClave]['texto'].$this->aFormulario['frmElementos'][$this->sClave]['campo']."\n";

		return($aElemento);
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

		//$this->aErrorMensajes = array(__METHOD__ => 'Tipo de validación (%s) no reconocido.');
	}

	protected function elemento(){
		$aElemento = $aCssTag = array();
		if(isset($this->aDatos['valor']) && is_array($this->aDatos['valor'])) $this->aDatos['valor'] = $this->aDatos['valor'][0]; //campos/zonas repetidos, el resto de los repetidos se crean con javascript (se construye el array con los datos necesarios en formulario::creaFormulario()

		//clases CSS, usadas tambien para validaciones
		$sCssLabel = $sSufijNombreCampo = '';
		if(!empty($this->aCss['label'])) $sCssLabel = ' class="'.$this->aCss['label'].'"';
		if(!empty($this->aCss[$this->aDatos['tipo']])) $aCssTag[] = $this->aCss[$this->aDatos['tipo']];
		if(!empty($this->aCss['dis']) && ($this->sActionHide == 'del' || $this->aDatos['longitud'] === 0)){
			$sSufijNombreCampo = $this->aCss['dis'];
			$aCssTag[] = $this->aCss['dis'];
		}
		if(!empty($this->aCss['obligatorio']) && !empty($this->aDatos['obligatorio']) && empty($sSufijNombreCampo)) $aCssTag[] = $this->aCss['obligatorio'];
		if(isset($this->aDatos['validacion'])) $aCssTag[] = $this->aDatos['validacion'];
		if(!empty($this->aDatos['isHTML'])) $aCssTag[] = 'isHTML';
		$sCssTag = count($aCssTag)?' class="'.implode(' ', $aCssTag).'"':'';

		$aElemento['texto'] = $this->aExtra[0].'<label for="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'"'.$sCssLabel.'>'.$this->aDatos['titulo'].'</label>'.$this->aExtra[1];
		$aElemento['campo'] = $this->aExtra[2].'<textarea id="'.$this->sClave.$this->sFormNombre.$sSufijNombreCampo.'" name="'.$this->sClave.$sSufijNombreCampo.'"'.$sCssTag.' tabindex="'.$this->iTabIndex.'"'.(!empty($this->aDatos[3])?' maxlength="'.$this->aDatos['longitud'].'"':'').(!empty($sSufijNombreCampo)?' disabled="disabled"':'').((!empty($this->aDatos['depende']) && (!empty($this->aDatos['depende']['hid']) && $this->aDatos['depende']['hid'] == 'tag') && $this->aDatos['depende']['val'] != $this->aDatos['depende']['val_op'])?' style="display:none;"':'').' rows="" cols="">'.(isset($this->aDatos['valor'])?$this->aDatos['valor']:'').'</textarea>'.$this->aExtra[3];
		if(!empty($sSufijNombreCampo)) $aElemento['campo'] .= '<input id="'.$this->sClave.$this->sFormNombre.'" name="'.$this->sClave.'" type="hidden" value="'.(isset($this->aDatos['valor'])?htmlspecialchars($this->aDatos['valor']):'').'" />';

		$aElemento['hint'] = empty($this->aDatos['hint'])?'':str_replace('aCssTags', (empty($this->aCss['hint'])?'':$this->aCss['hint']), (empty($this->sHintIco) || $this->sHintIco == '#'?$this->aDatos['hint']:str_replace('src="#"', 'src="'.$this->sHintIco.'"', $this->aDatos['hint'])));
		$aElemento['help'] = empty($this->aDatos['help'])?'':'<span'.(empty($this->aCss['help'])?'':' class="'.$this->aCss['help'].'"').'>'.$this->aDatos['help'].'</span>';

		return($aElemento);
	}
}
