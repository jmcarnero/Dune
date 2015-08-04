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
 * Generador de listados, con caja de busqueda, filtrado y paginacion
 *
 * Depende de class.ddbb.inc y class.rdatazo_*.inc
 * #core PHP 5
 * Ejemplo de uso:
 * <code>
include('class.listazo.inc');

$aCampos = array(
	'apellido_1' => array('titulo' => 'Apellidos', 'banderas' => 'bo'),
	'apellido_2' => array('banderas' => 'b'),
	'nombre' => array('banderas' => 'lb',
		'sublistazo' => array(
		'<strong>NIF:</strong> ' => 'NIF',
		'<strong>Nacionalidad:</strong> ' => '<?php echo muestraPais($field["nacionalidad"]); ?>',
		)
	),
	'tipo' => array('titulo' => '<strong>Tipo:</strong> ', 'campo' => '<?php echo $field["tipo"]; ?>', 'banderas' => 'lb', 'tipos' => array('Seleccione...' => '', '-Científico +Mantenimiento' => '-%Cien%%Mant', 'Tecnólogo' => 'Tecnólogo', 'Técnico' => 'Técnico', 'Mantenimiento' => 'Mantenimiento', 'Gestión' => 'Gestión')),
	'categoria_puesto' => array('titulo' => '<strong>Categoría del puesto:</strong> ', 'campo' => ' <?php echo $field["categoria_puesto"]; ?>', 'banderas' => 'lb'),
	'relacion_laboral' => array('titulo' => '<strong>Relacion laboral:</strong> ', 'campo' => '<?php echo $field["relacion_laboral"]; ?>', 'banderas' => 'lbo')
);

$oListado = new Listazo($aCampos);
$oListado->setParametrosUrl(array('wht='.$_REQUEST['wht'], 'opMenu='.$_REQUEST['opMenu']));
//$oListado->generar();
$oListado->pintar();
 * </code>
 *
 * @author José M. Carnero
 * @version 2
 * @since 2010-05-28
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Listazo
 */
class Listazo {

	/**
	 * Campos que se mostraran en el listado
	 *
	 * @var array
	 * @see Listazo::setCampos()
	 */
	protected $aCampos;

	/**
	 * Clase CSS base, se antepone a cualquier otra donde se necesite
	 *
	 * @var string
	 */
	protected $sCssBase;

	/**
	 * Elementos del formulario, ya construidos, que posteriormente seran pintados
	 *
	 * @var array
	 */
	protected $aElementosFormulario;

	/**
	 * Filas de datos a pintar
	 *
	 * @see Listazo::setFilas($aFilas)
	 * @var array
	 */
	protected $aFilas;

	/**
	 * Cantidad inicial de filas a mostrar
	 *
	 * @var integer
	 */
	protected $iFilasCant;

	/**
	 * Pagina de resultados
	 *
	 * @var integer
	 */
	protected $iFilasPag;

	/**
	 * Array con las distintas cantidades de filas a mostrar
	 *
	 * @var array
	 */
	protected $aFilasMostradas;

	/**
	 * Total de filas recuperadas en la consulta (sin limit)
	 *
	 * @var integer
	 */
	protected $iFilasTotal;

	/**
	 * Metodo del formulario
	 *
	 * @var string
	 */
	private $sFrmMetodo;

	/**
	 * Nuevos filtros para la caja de busqueda; se enviaran como parte del formulario
	 *
	 * @var array
	 * @see Listazo::setNuevoFiltro($forme)
	 */
	private $aNuevoFiltro;

	/**
	 * Campos de ordenacion, por si quiere pintarse personalizado
	 *
	 * @var array
	 */
	private $aOrderBy;

	/**
	 * Muestra u oculta paginacion
	 *
	 * @var boolean
	 */
	protected $bPaginacion;

	/**
	 * Parametros de la url, para conservar en paginacion
	 *
	 * @var array
	 * @see Listazo::getParametrosUrl(),Listazo::setParametrosUrl()
	 */
	private $aParamsUrl;

	/**
	 * URL que procesara las peticiones, tanto busqueda como paginacion; por defecto la propia pagina
	 *
	 * @var string
	 * @see Listazo::setUrl($url)
	 */
	private $sUrl;

	/**
	 * Recoge GET o POST segun el que se haya utilizado para llegar a esta pagina
	 *
	 * @var array
	 * @see Listazo::setVariablesRequest(),Listazo::getVariablesRequest()
	 */
	private $aVariablesRequest;

	/**
	 * Constructor
	 *
	 * @param array $aFilas Filas a listar
	 */
	function __construct($aFilas = array(), $aCampos = array()){
		$this->sCssBase = 'Listazo';

		$this->sFrmMetodo = 'post';
		$this->aRequest = '';
		$this->setVariablesRequest(); //recoge las variables para busqueda y paginacion
		$this->aParamsUrl = array();
		$this->aOrderBy = array();
		$this->aNuevoFiltro = array();

		$this->setFilasMostradas();
		$this->setFilas($aFilas);
		$this->setCampos($aCampos);
		$this->aElementosFormulario = array('busqueda' => array(), 'filas' => array(), 'paginacion' => array());
		$this->setUrl();
		$this->iFilasPag = 0;

		$this->setPaginacion(count($aFilas), 20);
	}

	/**
	 * Traduccion de textos
	 * Depende de class.L10n.inc
	 *
	 * @param string $sCadena Cadena a traducir
	 * @return string
	 */
	private function _t($sCadena){
		return $sCadena;
	}

	/**
	 * Evalua el codigo (html y php) que se le pase, usar eval_html($string)
	 *
	 * @param string $string Cadena a evaluar
	 * @return string
	 */
	private function eval_buffer($string){
		global $oResultados, $aGlobalListado, $op;

		ob_start();
		eval("$string[2]");
		$ret = ob_get_contents();
		ob_end_clean();

		return $ret;
	}

	/**
	 * Evalua el codigo (html y php) que se le pase
	 *
	 * @param string $string Cadena a evaluar
	 * @return string
	 */
	protected function eval_html($string){
		return preg_replace_callback("/(<\?php|<\?)(.*?)\?>/si", array(&$this, 'eval_buffer'), $string);
	}

	/**
	 * Devuelve un identificador para clases css segun la imagen/icono del fichero pasado
	 *
	 * @param string $file nombre.extension de fichero
	 * @return array
	 */
	//TODO ruta donde se encuentren los iconos configurable; no esta en uso, se puede aprovechar la parte "formulario" del array de campos para identificar los que la usen
	protected function fileIcons($file){
		$ext = explode('.', $file);
		$sTipo = 'ico';

		switch(strtolower(array_pop($ext))){
			/*audio*/
			case 'aif': case 'mp3': case 'wav':
				$sTipo .= 'Aud';
				break;
			/*imagenes*/
			case 'bmp': case 'gif': case 'jpg': case 'jpeg': case 'png': case 'tga': case 'tif': case 'tiff':
				$sTipo .= 'Img';
				break;
			/*comprimidos*/
			case 'ace': case 'arj': case 'lhz': case 'rar': case 'zip':
				$sTipo .= 'Zip';
				break;
			/*video*/
			case 'avi': case 'flv': case 'mpeg': case 'mpg': case 'wmv':
				$sTipo .= 'Vid';
				break;
			/*texto*/
			case 'doc'/*ms word*/: case 'docx'/*ms word >= 2007*/: case 'fodt'/*open document (plain xml)*/: case 'rtf'/*texto enriquecido*/: case 'txt'/*texto generico*/: case 'sxw'/*open office = 1*/: case 'sdw'/*starwriter*/:
				$sTipo .= 'Txt';
				break;
			/*hojas de calculo*/
			case 'csv'/*datos en texto plano*/: case 'fods'/*open document (plain xml)*/: case 'ods'/*open office*/: case 'sdc'/*starcalc*/: case 'sxc'/*open office = 1*/: case 'xls'/*ms excel*/: case 'xlsx'/*ms excel >= 2007*/:
				$sTipo .= 'Cal';
				break;
			case 'pdf': //pdf (portable document file)
				$sTipo .= 'Pdf';
				break;
			/*desconocido*/
			default:
				$sTipo .= 'Gen';
		}

		return $sTipo;
	}

	/**
	 * Cantidad de filas mostradas por pagina.
	 *
	 * @return boolean
	 */
	protected function filasMostradas(){
		if(isset($this->aVariablesRequest['ListazoBuscCant']) && $this->aVariablesRequest['ListazoBuscCant'] != ''){
			$this->iFilasCant = $this->aVariablesRequest['ListazoBuscCant'];
			$this->aParamsUrl[] = 'ListazoBuscCant=' . $this->aVariablesRequest['ListazoBuscCant'];
		}

		if(count($this->aFilasMostradas) > 1){
			$sTemp = '<div class="' . $this->sCssBase . ' numFilas"><label for="ListazoBuscCant">' . $this->_t('Ver') . ': </label><select id="ListazoBuscCant" name="ListazoBuscCant">';
			foreach($this->aFilasMostradas as $valor){
				$sTemp .= '<option value="' . $valor . '"' . ($this->iFilasCant==$valor ? ' selected="selected"' : '') . '>' . $this->_t($valor) . '</option>';
			}
			$sTemp .= '</select> ' . $this->_t('Resultados') . '</div>';
			$this->aElementosFormulario['busqueda'][] = $sTemp . "\n";
		}
		elseif(count($this->aFilasMostradas) == 1){
			$this->iFilasCant = $this->aFilasMostradas[0];
		}

		return true;
	}

	/**
	 * Genera el listado, con todas las condiciones que resulten de las opciones del array de campos; y prepara los elementos que se pintaran.
	 *
	 * @return boolean
	 */
	public function generar(){
		//caja de busqueda
		$this->palabrasClave();
		$this->tipo();
		$this->filasMostradas();
		$this->ordenacion();

		//toma la posicion de paginacion
		if(isset($this->aVariablesRequest['ListazoRowPos']) && $this->aVariablesRequest['ListazoRowPos'] != '')
			$this->iFilasPag = $this->aVariablesRequest['ListazoRowPos'];

		//paginacion
		$this->paginacion();

		return true;
	}

	/**
	 * Recupera los actuales parametros de la URL.
	 *
	 * @since 2010-11-19
	 * @param boolean $iniciales Recupera solo los parametros no construidos por la propia clase (true) o todos (false)
	 * @param string $amp Separador de campos de la URL //TODO si la URL se cambia aqui deberia ser cambiable (y estar sincronizado con $this->paginacion() )
	 * @return boolean
	 */
	public function getParametrosUrl($iniciales = true, $amp = '&'){
		$aRet = array();

		if($iniciales){
			foreach($this->aParamsUrl as $valor){
				if(strpos($valor, 'ListazoBuscKeyword') === false && strpos($valor, 'ListazoBuscTipo') === false && strpos($valor, 'ListazoBuscCant') === false && strpos($valor, 'ListazoOrderBy') === false)
					$aRet[] = $valor;
			}
		}
		else $aRet = $this->aParamsUrl;

		return implode($amp, $aRet);
	}

	/**
	 * Devuelve las variables recuperadas en busqueda.
	 *
	 * @return string
	 */
	public function getVariablesRequest(){
		return $this->aVariablesRequest;
	}

	/**
	 * Opciones de ordenacion
	 *
	 * @return boolean
	 */
	protected function ordenacion(){
		$aCampos = array(); //campos de la base de datos con los que se ordenara
		$this->aVariablesRequest['ListazoOrderBy'] = isset($this->aVariablesRequest['ListazoOrderBy']) ? $this->aVariablesRequest['ListazoOrderBy'] : '';

		foreach($this->aCampos as $clave => $valor){
			if(strpos(strtolower($valor['banderas']), 'o') !== false)
				$aCampos[$clave] = (isset($valor['titulo'])) ? $valor['titulo'] : '';
		}
		if(empty($aCampos))
			return true; //ningun campo en el que buscar, no se pinta este elemento

		$this->aOrderBy = $aCampos; //array de los campos de ordenacion, por si quiere pintarse sin ayuda de la clase

		//ordenacion
		if(isset($this->aVariablesRequest['ListazoOrderBy']) && $this->aVariablesRequest['ListazoOrderBy'] != '' && isset($this->aOrderBy[$this->aVariablesRequest['ListazoOrderBy']])){
			//$this->sOrderBy .= ', ' . $this->aVariablesRequest['ListazoOrderBy'];
			$this->aParamsUrl[] = 'ListazoOrderBy=' . $this->aVariablesRequest['ListazoOrderBy'];
		}

		$sTemp = '<div class="' . $this->sCssBase . ' ordenacion"><strong>' . $this->_t('Ordenado por') . ':</strong><br />' . "\n";
		$sTemp .= '<input type="radio" name="ListazoOrderBy"' . ($this->aVariablesRequest['ListazoOrderBy'] == '' ? ' checked="checked"' : '') . ' value=""/>&nbsp;<label>' . $this->_t('Sin orden') . '</label>' . "\n";
		foreach($aCampos as $clave => $valor){
			$sTemp .= '<span>|</span>&nbsp;&nbsp;<input type="radio" name="ListazoOrderBy"' . ($this->aVariablesRequest['ListazoOrderBy'] == $clave ? ' checked="checked"' : '') . ' value="' . $clave . '"/>&nbsp;&nbsp;<label>' . $this->_t(strip_tags($valor != '' ? $valor : $clave)) . '</label>&nbsp;&nbsp;' . "\n";
		}
		$sTemp .= '</div>' . "\n";
		$this->aElementosFormulario['busqueda'][] = $sTemp . "\n";

		return true;
	}

	/**
	 * Devuelve un array con los campos de busqueda ('campo' => 'titulo')
	 *
	 * @since 2010-11-19
	 * @return array
	 */
	public function orderByElems(){
		return $this->aOrderBy;
	}

	/**
	 * Paginacion.
	 *
	 * @return array
	 */
	protected function paginacion(){
		if(!empty($this->aElementosFormulario['paginacion'])) //elemento ya construido
			return true;

		if($this->iFilasTotal > $this->iFilasCant || $this->bPaginacion){
			//parametros de la URL
			/*if(count($this->aParamsUrl)) $sParamsUrl = implode('&', $this->aParamsUrl).'&';
			else $sParamsUrl = '';*/
			$sUrl = $this->getParametrosUrl(false);
			$sUrl = $this->sUrl . (empty($sUrl) ? '' : $sUrl . '&');

			//anteriores
			if($this->iFilasPag>0){
				$this->aElementosFormulario['paginacion'][] = '<a href="' . $sUrl . 'ListazoRowPos=' . ($this->iFilasPag - $this->iFilasCant) . '">&lt;&lt;&nbsp;' . $this->_t('previos') . '</a>' . "\n";
			}

			//seleccionable y total
			//TODO convertir evento (onchange) a no obstrusivo
			//TODO traduccion de textos visibles
			$this->aElementosFormulario['paginacion'][] = $this->_t('página') . ' <select id="Listazo_paginacion_pags" name="Listazo_paginacion_pags" onchange="window.location.href=\'' . $sUrl . 'ListazoRowPos=\' + this.options[this.selectedIndex].value;">' . "\n";
			$iTotalPags = ceil($this->iFilasTotal / $this->iFilasCant);
			for($i = 0; $i <= $iTotalPags-1; $i++){
				$this->aElementosFormulario['paginacion'][] = '<option value="' . ($i * $this->iFilasCant) . '"' . ($i == $this->iFilasPag / $this->iFilasCant ? ' selected="selected"' : '') . '>' . ($i + 1) . '</option>' . "\n";
			}
			$this->aElementosFormulario['paginacion'][] = '</select> ' . $this->_t('de') . ' ' . $iTotalPags . "\n";

			//siguientes
			if(($this->iFilasPag+$this->iFilasCant) <= $this->iFilasTotal){
				$this->aElementosFormulario['paginacion'][] = '<a href="' . $sUrl . 'ListazoRowPos=' . ($this->iFilasPag + $this->iFilasCant) . '">' . $this->_t('siguientes') . '&nbsp;&gt;&gt;</a>' . "\n";
			}
		}

		return true;
	}

	/**
	 * Crea una caja para busqueda por palabras clave.
	 *
	 * @return array
	 */
	protected function palabrasClave(){
		$aCampos = array(); //campos de la base de datos en los que se buscara
		$this->aVariablesRequest['ListazoBuscKeyword'] = isset($this->aVariablesRequest['ListazoBuscKeyword']) ? $this->aVariablesRequest['ListazoBuscKeyword'] : '';

		foreach($this->aCampos as $clave => $valor){
			if(strpos(strtolower($valor['banderas']), 'b') !== false)
				$aCampos[] = $clave;
		}
		if(empty($aCampos)) //ningun campo en el que buscar, no se pinta este elemento
			return true;

		if(isset($this->aVariablesRequest['ListazoBuscKeyword']) && $this->aVariablesRequest['ListazoBuscKeyword'] != ''){
			$this->aParamsUrl[] = 'ListazoBuscKeyword=' . $this->aVariablesRequest['ListazoBuscKeyword'];
		}

		$this->aElementosFormulario['busqueda'][] = '<div class="' . $this->sCssBase . ' palabrasClave"><label for="ListazoBuscKeyword">' . $this->_t('Palabras clave') . ': </label><input id="ListazoBuscKeyword" type="text" name="ListazoBuscKeyword" value="' . $this->aVariablesRequest['ListazoBuscKeyword'] . '" /></div>' . "\n";

		return true;
	}

	/**
	 * Pinta el listado, con seccion de busqueda, listado en si y paginacion.
	 * Sobreescribible en clases hijas; también se puede ignorar si se van llamando los métodos que pintan cada parte.
	 *
	 * @param boolean $echo Indica si se escribira (true, por defecto) o devolvera (false) el resultado
	 * @return string
	 */
	public function pintar($echo = true){
		//caja de busqueda
		$sBusqueda = $this->pintarBusqueda(false);

		//listado
		$sListado = $this->pintarListado(false);

		//paginacion
		$sPaginacion = $this->pintarPaginacion(false);

		if($echo){
			echo $sBusqueda . $sListado . $sPaginacion;
			return true;
		}
		else
			return $sBusqueda . $sListado . $sPaginacion;
	}

	/**
	 * Contruye la caja de busqueda del listado, html
	 * Sobreescribible en clases hijas
	 *
	 * @param boolean $echo Indica si se escribira (true, por defecto) o devolvera (false) el resultado
	 * @return string
	 */
	public function pintarBusqueda($echo = true){
		if(empty($this->aElementosFormulario['busqueda']))
			$this->generar();

		$sRet = '';
		$sUrl = $this->getParametrosUrl(true);
		$sUrl = $this->sUrl . $sUrl;

		if(!empty($this->aElementosFormulario['busqueda'])){ //cajas de busqueda
			$sRet .= '<div id="Listazo_busqueda"><form id="Listazo_busqueda_form" action="' . $sUrl . '" method="' . $this->sFrmMetodo . '">';
			$sRet .= implode('', $this->aElementosFormulario['busqueda']) . "\n";

			if(!empty($this->aNuevoFiltro) && is_array($this->aNuevoFiltro))
				$sRet .= implode('', $this->aNuevoFiltro) . "\n";

			$sRet .= '<input type="submit" value="' . $this->_t('Enviar') . '" name="submit" /></form></div>' . "\n";
		}

		if($echo){
			echo $sRet;
			return true;
		}
		else
			return $sRet;
	}

	/**
	 * Construye el html para el listado.
	 * Sobreescribible en clases hijas
	 *
	 * @todo pueden automatizarse las traducciones aqui?
	 * @param boolean $echo Indica si se escribira (true, por defecto) o devolvera (false) el resultado
	 * @return string
	 */
	public function pintarListado($echo = true){
		$sTemp = '';
		$sTemp .= '<div id="Listazo_filas">' . "\n";

		if(!empty($this->aFilas)){
			$iNumFilas = count($this->aFilas);

			foreach($this->aFilas as $clave => $valor){ //recorre todas las filas
				$sTempClass = ' class="' . $this->sCssBase . ' ';
				$sTempClass .= (!$clave) ? 'fila_primera ' : '';
				$sTempClass .= ($clave % 2) ? 'fila_par' : 'fila_impar';
				$sTempClass .= (($clave + 1) == $iNumFilas) ? ' fila_ultima' : '';
				$sTempClass .= '"';

				$sTemp .= '<div id="Listazo_fila_' . $clave . '"' . $sTempClass . '>' . "\n";
				foreach($this->aCampos as $clavef => $valorf){ //recorre los campos de cada fila
					if(strpos(strtolower($valorf['banderas']), 'l') === false)
						continue;

					$sTemp .= '<p id="' . $clavef . '_' . $clave . '">';

					//campos evaluados
					$sTitulo = empty($valorf['titulo']) ? '' : $valorf['titulo'];
					if(strpos($sTitulo, '<?php') !== false){ //titulo evaluado
						$sTitulo = $this->eval_html(str_replace('$field', '$this->aFilas[' . $clave . ']', $sTitulo));
					}

					$sCampo = empty($valorf['campo']) ? (empty($valor[$clavef]) ? '' : $valor[$clavef]) : $valorf['campo'];
					if(strpos($sCampo, '<?php') !== false){ //campo evaluado
						$sCampo = $this->eval_html(str_replace('$field', '$this->aFilas[' . $clave . ']', $sCampo));
					}

					if(!empty($sCampo))
						$sTemp .= $sTitulo . (empty($valorf['visibleCars']) ? $sCampo : $this->recortarCad($sCampo, $valorf['visibleCars']));

					$sTemp .= '</p>' . "\n";
					if(!empty($valorf['sublistazo']))
						$sTemp .= $this->sublistado($valorf['sublistazo'], $clave);
				}
				$sTemp .= '</div>' . "\n";
			}
		}
		$sTemp .= '</div>' . "\n";

		if($echo){
			echo $sTemp;
			return true;
		}
		else
			return $sTemp;
	}

	/**
	 * Construye la paginacion, html.
	 * Sobreescribible en clases hijas
	 *
	 * @param boolean $echo Indica si se escribira (true, por defecto) o devolvera (false) el resultado
	 * @return string
	 */
	public function pintarPaginacion($echo = true){
		if(empty($this->aElementosFormulario['paginacion']))
			$this->generar();

		$sTemp = '';

		$sTemp .= '<div id="Listazo_paginacion">';
		$sTemp .= implode('', $this->aElementosFormulario['paginacion']) . "\n";
		$sTemp .= '</div>';

		if($echo){
			echo $sTemp;
			return true;
		}
		else
			return $sTemp;
	}

	/**
	 * Recorta la cadena "$cad" a "$long" caracteres, sin cortar palabras.
	 * si "$punto" la recorta aun mas, hasta el primer "$punto" (un solo caracter) desde el final de la cadena (para no cortar frases
	 *
	 * @todo Excluir tags HTML "strip_tags($cad, '<a>')" deja "<a>"; controlar que no quede ninguno abierto despues de cortar la cadena (o se arrastra el tag a las demas noticias mostradas)
	 * @since 2012-03-12
	 * @param string $cad Cadena a cortar
	 * @param integer $long Maximo de caracteres a conservar
	 * @param string $punto Caracter de corte, si no se indica se busca el ultimo espacio
	 * @return string
	 */
	protected function recortarCad($cad, $long, $punto = false){
		if(strlen($cad) <= $long)
			return $cad;

		$sFinal = '&hellip;'; //&hellip; entidad de puntos suspensivos
		//$long -= strlen($sFinal);
		$cadCort = substr(strip_tags($cad), 0, $long);
		if($punto === false) $cadCort = substr($cadCort, 0, strrpos($cadCort, ' '));
		else $cadCort = substr($cadCort, 0, strrpos($cadCort, $punto));

		return $cadCort . $sFinal;
	}

	/**
	 * Suma una o mas filas al array de campos que define las filas del listado
	 * (se ignoran las filas que no tengan 'banderas', ya que el array puede ser compartido con class.formulario.inc)
	 *
	 * cada elemento del array se define como:
	 * 'nombre_de_campo' => array(
	 * 	'titulo' => 'Título del campo', //opcional
	 * 	'campo' => '$field["nombre_de_campo"]', //opcional; necesario si el campo incorpora html o php; si no esta presente se toma la clave del elemento, que es el nombre del propio campo
	 * 	'banderas' => 'blo',
	 * 	'sublistazo' => array('Título' => 'campo_o_evaluado', 'Título 2' => 'campo_o_evaluado'), //opcional
	 * 	'tipos' => array('Seleccione ...' => '', 'Título de la opcion' => 'valor', 'Título de la opcion 2' => 'valor'), //opcional
	 * 	'visibleCars' => 'numero maximo de caracteres que se mostraran, si supera esta cantidad se corta en la palabra mas proxima y se añade [...]' //opcional
	 * )
	 * banderas:
	 * b -> se haran busquedas en este campo
	 * l -> se muestra en el listado
	 * o -> campo por el que podra ordenarse el listado
	 *
	 * Tanto en 'titulo' como en 'campo' se puede poner HTML y codigo PHP (entre <?php y ?>) que sera evaluado,
	 * se puede usar "$field[nombre_de_campo]" para referenciar campos del propio listado (sustituyendose por el valor de la fila en curso)
	 *
	 * @todo campos en xml (como formulario)
	 * @param array $aCampos Campo/s de la base de datos en los que se buscara
	 * @return boolean
	 */
	public function setCampos($aCampos = array()){
		if(empty($aCampos)){ //TODO tomar las claves de $this->aFilas por defecto
			return false;
		}

		foreach($aCampos as $clave => $valor){ //'banderas' es exclusivo de esta clase, si un campo no lo lleva se ignora el campo
			if(isset($valor['banderas']))
				$this->aCampos[$clave] = $valor;
		}

		if(count($this->aCampos))
			return true;
		else{ //ningun campo utilizable
			trigger_error('Sólo se mostrarán filas vacías, el array de campos no contiene ninguno válido', E_USER_WARNING);
			return false;
		}
	}

	/**
	 * Asigna las filas que se mostraran
	 * array(
	 * 	array('campo1' => 'valor0-1', 'campo2' => 'valor0-2'),
	 * 	array('campo1' => 'valor1-1', 'campo2' => 'valor1-2'),
	 * 	array('campo1' => 'valor2-1', 'campo2' => 'valor2-2'),
	 * 	[...]
	 * )
	 *
	 * @param array $aFilas Filas a listar
	 */
	public function setFilas($aFilas = array()){
		$this->aFilas = $aFilas;

		if(empty($this->iFilasTotal))
			$this->iFilasTotal = count($this->aFilas);
	}

	/**
	 * Asigna las cantidades de filas que se devolveran en los resultados.
	 * Por defecto: 10, 20, 50, 100 y 200
	 * si se pasa un solo valor (array o no), no se pinta el desplegable de numeros de resultados
	 * Devuelve falso si hay errores (como un valor no numerico)
	 *
	 * @param array $filas Cantidades a mostrar; se pintaran en el desplegable en el mismo orden; o un solo numero, no array
	 * @return boolean
	 */
	public function setFilasMostradas($filas = array(5, 10, 20, 50)){
		$this->aFilasMostradas = array();

		if(is_array($filas)){
			foreach($filas as $valor){
				if(!is_numeric($valor)){
					trigger_error('Lista de valores para selección de cantidades de filas contiene al menos un dato no permitido [' . $valor . ']; todos los valores deben ser numéricos', E_USER_WARNING);
					return false;
				}
				$this->aFilasMostradas[] = (int) round($valor);
			}
		}
		else{
			if(!is_numeric($filas)){
				trigger_error('Lista de valores para selección de cantidades de filas contiene al menos un dato no permitido [' . $filas . ']; todos los valores deben ser numéricos', E_USER_WARNING);
				return false;
			}
			$this->aFilasMostradas[] = (int) round($filas);
		}

		return true;
	}

	/**
	 * Permite incluir nuevos filtros en la caja de busqueda, como parte del formulario
	 *
	 * @since 2012-05-07
	 * @param array $forme Elementos de formulario (solo el elemento, con label, select, input o lo que corresponda, pero sin <form>) a añadir al de busqueda
	 * @return boolean
	 */
	public function setNuevoFiltro($forme = false){
		if(empty($forme))
			return false;

		$this->aNuevoFiltro[] = $forme;
		return true;
	}

	/**
	 * Asigna los parametros iniciales para la URL.
	 * Uno solo o array; cada elemento debe ser: 'parametro1=valor_de_parametro1'
	 *
	 * @param array $campos Parametros por defecto de la URL
	 * @return boolean
	 */
	public function setParametrosUrl($campos = array()){
		if(empty($campos))
			return false;

		if(is_array($campos)){
			foreach($campos as $valor) $this->aParamsUrl[] = $valor;
		}
		else $this->aParamsUrl[] = $campos;

		return true;
	}

	/**
	 * Asigna la paginacion: numero total de resultados y cantidad por pagina
	 *
	 * @param integer $iTotal Total de resultados
	 * @param integer $iCantidad Cantidad (maxima) a mostrar por pagina
	 * @param boolean $bPaginacion Si true construira paginacion, si false no la construira
	 * @return boolean
	 */
	public function setPaginacion($iTotal = null, $iCantidad = null, $bPaginacion = true){
		$this->iFilasCant = $iCantidad;
		$this->iFilasTotal = $iTotal;
		$this->bPaginacion = $bPaginacion;
	}

	/**
	 * Asigna la URL para paginación y búsquedas.
	 *
	 * @todo verificar url que pase el usuario
	 * @since 2012-07-14
	 * @param string $url Url que se usara para formularios y enlaces
	 * @return string
	 */
	public function setUrl($url = false){
		if($url == false)
			$this->sUrl = $_SERVER['REQUEST_URI'];

		if(strpos($this->sUrl, '?') !== false)
			$this->sUrl = substr($this->sUrl, 0, strpos($this->sUrl, '?'));

		$this->sUrl .= '?';

		return true;
	}

	/**
	 * Recoge las variables para busqueda.
	 *
	 * @return string
	 */
	private function setVariablesRequest(){
		$this->aVariablesRequest = array();

		if($_SERVER['REQUEST_METHOD'] == 'GET') $aTemp = &$_GET;
		elseif($_SERVER['REQUEST_METHOD'] == 'POST') $aTemp = &$_POST;
		else $aTemp = &$_REQUEST;

		if(isset($aTemp['ListazoBuscKeyword'])) //palabra a buscar
			$this->aVariablesRequest['ListazoBuscKeyword'] = $aTemp['ListazoBuscKeyword'];
		if(isset($aTemp['ListazoBuscTipo'])) //busqueda por tipos
			$this->aVariablesRequest['ListazoBuscTipo'] = $aTemp['ListazoBuscTipo'];
		if(isset($aTemp['ListazoBuscCant'])) //cantidad de filas a mostrar
			$this->aVariablesRequest['ListazoBuscCant'] = $aTemp['ListazoBuscCant'];
		if(isset($aTemp['ListazoOrderBy'])) //criterio de ordenacion
			$this->aVariablesRequest['ListazoOrderBy'] = $aTemp['ListazoOrderBy'];
		if(isset($aTemp['ListazoRowPos'])) //posicion de paginacion
			$this->aVariablesRequest['ListazoRowPos'] = $aTemp['ListazoRowPos'];

		return true;
	}

	/**
	 * Crea los sublistados. En los campos evaluados se usa "$field[campo]" referencia para los campos recuperados en la consulta
	 *
	 * @param array $sublistado Elementos del sublistado
	 * @param integer $pos Posicion del array de resultados
	 * @return string
	 */
	protected function sublistado($sublistado, $pos = 0){
		if(empty($sublistado))
			return '';

		$sTemp = '<ul class="' . $this->sCssBase . ' sublistazo">';

		foreach($sublistado as $clave => $valor){
			$sTitulo = empty($clave) ? '' : $clave;
			if(strpos($sTitulo, '<?php') !== false){ //titulo evaluado
				$sTitulo = $this->eval_html(str_replace('$field', '$this->aFilas[' . $pos . ']', $sTitulo));
			}

			$sCampo = empty($valor)?'':$valor;
			if(strpos($sCampo, '<?php') !== false){ //campo evaluado
				$sCampo = $this->eval_html(str_replace('$field', '$this->aFilas[' . $pos . ']', $sCampo));
			}

			if(!empty($sTitulo) || !empty($sCampo))
				$sTemp .= '<li>' . $sTitulo . $sCampo . '</li>';
		}
		$sTemp .= '</ul>';

		return $sTemp;
	}

	/**
	 * Busqueda por tipo.
	 * Si la clase de recuperacion de datos lo permite,
	 * debe ser posible incluir varios terminos de filtrado (en cualquier caso deben ir en un array parte de los valores del elemento, 'tipos' => array('Título de la opcion' => 'valor', ...),
	 * separados por '%%' -> LIKE, separados por '-%' -> NOT LIKE
	 * ej: 'tipos' => array('Imagenes' => '%%.bmp%%.jpg%%.jpeg%%.png', 'Comprimidos' => '-%.ace-%.rar-%.zip', 'Video' => '.mpg');
	 *
	 * @return boolean
	 */
	protected function tipo(){
		$aCampos = array(); //opciones de tipo que se buscaran

		foreach($this->aCampos as $clave => $valor){
			if(!empty($valor['tipos'])) $aCampos[] = $clave;
		}

		if(empty($aCampos))
			return true; //ningun campo de tipo, no se pinta este elemento

		for($i = 0;$i < count($aCampos); $i++){
			if(isset($this->aVariablesRequest['ListazoBuscTipo'.$i]) && $this->aVariablesRequest['ListazoBuscTipo'.$i] != ''){
				$this->aParamsUrl[] = 'ListazoBuscTipo' . $i . '=' . $this->aVariablesRequest['ListazoBuscTipo' . $i];
			}

			$sTitulo = isset($this->aCampos[$aCampos[$i]]['titulo']) ? $this->aCampos[$aCampos[$i]]['titulo'] : $aCampos[$i];
			$sTemp = '<div class="' . $this->sCssBase . ' tipo"><label for="ListazoBuscTipo' . $i . '">' . $this->_t($sTitulo) . '</label><select id="ListazoBuscTipo' . $i . '" name="ListazoBuscTipo' . $i . '">' . "\n";
			//$sTemp .= '<option value="">Seleccione ...</option>' . "\n";
			foreach($this->aCampos[$aCampos[$i]]['tipos'] as $clave => $valor){
				$sVarRequest = isset($this->aVariablesRequest['ListazoBuscTipo' . $i]) ? $this->aVariablesRequest['ListazoBuscTipo' . $i] : '';
				$sTemp .= '<option value="' . $valor . '"' . ($sVarRequest == $valor ? ' selected="selected"' : '') . '>' . $this->_t($clave) . '</option>' . "\n";
			}
			$sTemp .= '</select></div>' . "\n";
			$this->aElementosFormulario['busqueda'][] = $sTemp . "\n";
		}
		return true;
	}

}

/*
Lista de expresiones traducibles (mediante class.l10n.inc):
de|de
Ordenado por|Ordenado por
página|página
Palabras clave|Palabras clave
previos|previos
Resultados|Resultados
siguientes|siguientes
Sin orden|Sin orden
*/
