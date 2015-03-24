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
 * Capa de abstracción para recuperación de datos de distintas fuentes con una única interfaz
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package REdAtazo
 */

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta.
defined('CLASSES_DIR') or define('CLASSES_DIR',dirname(__FILE__));
include_once (CLASSES_DIR . '/interface.redatazo.inc');
include_once (CLASSES_DIR . '/class.importarCsv.inc');

/**
 * Clase para recuperar datos de archivos de texto, valores separados por coma (CSV)
 * Depende de 'class.importarCsv.inc'
 *
 * @since 2015-03-12
 */
class Redatazo_csv implements iRedatazo {
	/**
	 * Campos a importar del CSV, en el mismo orden que aparezcan; los campos "--" provocan que se ignore la columna que corresponda
	 *
	 * @var array
	 * @access private
	 */
	private $aCampos;

	/**
	 * Fila con los nombres de los campos, podrian ser los mismos que titulos del CSV, en cuyo caso debera ignorarse esta fila para importar datos ($this->iFilaInicial = $this->iFilaTitulos + 1); por defecto a false (no hay fila de titulos)
	 *
	 * @var integer
	 * @access private
	 */
	private $iFilaTitulos;

	/**
	 * Separador de valores en cada fila del CSV; si es un caracter escapado debe ir entre dobles comillas; por defecto ','
	 *
	 * @var string
	 * @access private
	 */
	private $sSeparadorCampos;

	/**
	 * Caracter que encierra a los valores a recuperar; por defecto '"'
	 *
	 * @var string
	 * @access private
	 */
	private $sCercoCampos;

	/**
	 * Ruta y nombre del fichero CSV
	 *
	 * @var string
	 * @access private
	 */
	private $sFichero;

	/**
	 * Objeto de importacion CSV (class.importarCsv)
	 *
	 * @var object
	 * @access private
	 */
	private $oCsv;

	/**
	 * Total de filas, sin limit
	 *
	 * @var integer
	 * @see rdatazo_csv::totalFilas($limit)
	 */
	public $iTotalFilas;

	/**
	 * Constructor
	 */
	function __construct($aDatos = array()){
		$this->setParametros($aDatos);

		$this->iFilasPag = $this->iFilasCant = false;

		$this->iTotalFilas = 0;

		$this->sError = false;

		$this->aCampos = array();

		$this->conectar();
	}

	/*function __destruct(){
		$this->oDDBB->desconectar();
	}*/

	/**
	 * Conectar con la clase CSV
	 *
	 * @todo eliminar esta conexion y hacer una nueva si cambian parametros, $this->setParametros
	 * @access private
	 * @return boolean
	 */
	private function conectar(){
		$this->oCsv = new importarCsv();
		if(!empty($this->aCamposVal)) $this->oCsv->aCamposVal = $this->aCamposVal;
		if(!empty($this->aCamposSepMultiple)) $this->oCsv->aCamposSepMultiple = $this->aCamposSepMultiple;
		if(!empty($this->aCamposExtra)) $this->oCsv->aCamposExtra = $this->aCamposExtra;
		if(!empty($this->iFilaTitulos)) $this->oCsv->iFilaTitulos = $this->iFilaTitulos;
		if(!empty($this->sSeparadorCampos)) $this->oCsv->sSeparadorCampos = $this->sSeparadorCampos;
		if(!empty($this->sCercoCampos)) $this->oCsv->asCercoCampos = $this->sCercoCampos;
		if(!empty($this->sFormatoFechas)) $this->oCsv->sFormatoFechas = $this->sFormatoFechas;
		if(!empty($this->aTraducciones)) $this->oCsv->aTraducciones = $this->aTraducciones;

		return true;
	}

	/**
	 * Ejecucion de consulta
	 * del archivo CSV
	 *
	 * @todo pasar $numFilas y $numPagina a $this->limit; teniendo en cuenta no pisar si no se pasa nada
	 * @access public
	 * @return array
	 */
	public function consulta($consulta = null, $parametros = array(), $numFilas = null, $numPagina = null){
		if(empty($this->aCampos) || empty($this->sFichero)){
			$this->sError .= sprintf($this->aErrorMensajes['recupera_datos_csv::consulta1']);
			return false; //si no hay fichero ni campos a importar se corta el proceso
		}

		$this->oCsv->aCampos = $this->aCampos;

		if(!empty($this->iFilasPag) && !empty($this->iFilasCant)){
			$this->oCsv->iFilaInicial = $this->iFilasPag * $this->iFilasCant;
			$this->oCsv->iFilaFinal = $this->iFilasCant + ($this->iFilasPag * $this->iFilasCant);
		}
		$this->oCsv->sFichero = $this->sFichero;
		if(!$this->oCsv->leeCsv()){
			$this->sError .= sprintf($this->aErrorMensajes['recupera_datos_csv::consulta2'], $this->oCsv->sError);
			return false; //no se ha podido leer el fichero en la clase importarCsv o algun otro error
		}

		//filtrar y ordenar resultados
		if(!empty($this->aWhere)) $this->oCsv->aResultados = $this->where($this->oCsv->aResultados);
		//if(!empty($this->aGroupBy)) $this->oCsv->aResultados = $this->groupBy($this->oCsv->aResultados); //TODO no agrupa bien, revisar funcionamiento
		if(!empty($this->sOrderBy)) $this->oCsv->aResultados = $this->orderBy($this->oCsv->aResultados);

		$this->limpiaConsulta(); //despues de lanzar una consulta debe limpiarse para empezar de cero la siguiente
		return true;
	}

	/**
	 * Borra una o mas filas
	 *
	 * @access public
	 * @param string $fichero Fichero en el que se hara el borrado
	 * @param array $condicion Condicion de borrado, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function delete($fichero, $condicion = array()){
		return false;
	}

	/**
	 * Fichero del que recuperar resultados (cuando CSV, XML, etc.)
	 *
	 * @access public
	 * @param string $fichero Fichero del que recuperar datos, ruta/nombre_de_fichero.csv
	 * @return boolean
	 */
	public function from($fichero){
		if(!is_readable($fichero))
			return false;

		$this->sFichero = realpath($fichero);
	}

	/**
	 * Devuelve la consulta lanzada
	 *
	 * @access public
	 * @param string $posicion Que consulta mostrar, por defecto la ultima lanzada
	 * @return string
	 */
	public function getConsulta($posicion = 'ultima'){
		return false;
	}

	/**
	 * Devuelve las filas recuperadas en la consulta
	 *
	 * @access public
	 * @return array
	 */
	public function getFilas(){
		return($this->oCsv->aResultados);
	}

	/**
	 * Inserta una fila
	 *
	 * @access public
	 * @param string $fichero Fichero en el que se hara la insercion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function insert($fichero, $datos){
		return false;
	}

	/**
	 * Ficheros con los que hacer join del principal
	 *
	 * @access public
	 * @param string $fichero Fichero y seguido de condicion (si no se pone en el segundo parametro)
	 * @param string $condicion Condicion del join (-opcional-)
	 * @param string $tipo Tipo de join: inner, left, right
	 * @return boolean
	 */
	public function join($fichero, $condicion = null, $tipo = 'inner'){
		return false;
	}

	/**
	 * Limita el numero de filas devuelto
	 *
	 * @access public
	 * @param integer $cantidad Cantidad de filas a devolver
	 * @param integer $offset Numero de fila desde la cual empezar a devolver
	 * @return boolean
	 */
	public function limit($cantidad, $offset = 0){
		/*$this->iNumeroFilas = $cantidad;
		$this->iOffsetFilas = $offset;
		return true;*/
		return false;
	}

	private function limpiaConsulta(){
		$this->sFichero = '';
		$this->aCampos = array();
	}

	/**
	 * Opciones de ordenacion
	 *
	 * @todo poner nombre de tabla y campos entre ` (tildes graves); decidir si el usuario puede enviar las palabras clave "order by"
	 * @access public
	 * @param string $campo Campo por el que se ordenaran los resultados; sin orden por defecto
	 * @param string $sentido Sentido de la ordenacion: asc(endente) o desc(endente)
	 * @return boolean
	 */
	public function order_by($campo, $sentido = 'asc'){
		return false;
	}

	/**
	 * Campos a devolver
	 *
	 * @access public
	 * @param string $campos Campos a devolver, separados por comas
	 * @return boolean
	 */
	public function select($campos = '*'){
		$aCampos = array_map('trim', explode(',', $campos));
		$this->aCampos = array_merge($this->aCampos, $aCampos);
	}

	/**
	 * Prepara los parametros que se utilizaran en la recuperacion de datos.
	 * $aDatos:
	 * 'aCampos' => array con los campos que contiene el CSV, en el mismo orden que aparezcan; los campos "--" provocan que se ignore la columna que corresponda
	 * 'aCamposVal' => Validaciones de campos. Vacio (por defecto) no hay validaciones, o solo los campos necesarios; asociativo: 'campo' => 'tipo de dato'
	 * 'CamposSepMultiple' => separadores para campos que contengan varios datos; 'nombre del campo' => 'separador'; se utiliza para traducciones
	 * 'aCamposExtra' => array asociativo con campos extra no contenidos en el CSV; en la forma 'nombreCampo' => 'valor'
	 * 'iFilaTitulos' => fila con los nombres de los campos, podrian ser los mismos que titulos del CSV, en cuyo caso debera ignorarse esta fila para importar datos ($this->iFilaInicial = $this->iFilaTitulos + 1); por defecto a false (no hay fila de titulos)
	 * 'sSeparadorCampos' => separador de valores en cada fila del CSV; si es un caracter escapado debe ir entre dobles comillas; por defecto ,
	 * 'sCercoCampos' => caracter que encierra a los valores a recuperar; por defecto "
	 * 'sFormatoFechas' => cadena con el formato de las fechas, ej.: 'aaaa-dd-mm'
	 * 'aTraducciones' => array de traducciones, donde las claves son los terminos que deban aparecer en el CSV y sus valores las traducciones; si esta vacio se ignora la traduccion; si una clave apunta a un array, la clave sera el nombre del campo al que afectan las traducciones (exclusivamente)
	 * 'sFichero' => nombre del fichero que contiene los datos CSV, ruta completa
	 *
	 * @access public
	 * @param array $aDatos Asociativo con los datos requeridos por la clase
	 * @return boolean
	 */
	public function setParametros($aDatos = array()){
		if(empty($aDatos)) return(false);

		//$this->sMotor = empty($aDatos['motor'])?'mysql':$aDatos['motor'];

		$this->aCamposVal = empty($aDatos['camposVal']) ? array() : $aDatos['camposVal'];
		$this->aCamposSepMultiple = empty($aDatos['camposSepMultiple']) ? array() : $aDatos['camposSepMultiple'];
		$this->aCamposExtra = empty($aDatos['camposExtra']) ? array() : $aDatos['camposExtra'];
		$this->iFilaTitulos = empty($aDatos['filaTitulos']) ? 0 : $aDatos['filaTitulos'];
		$this->sSeparadorCampos = empty($aDatos['separadorCampos']) ? ',' : $aDatos['separadorCampos'];
		$this->sCercoCampos = empty($aDatos['cercoCampos']) ? '"' : $aDatos['cercoCampos'];
		$this->sFormatoFechas = empty($aDatos['formatoFechas']) ? 'aaaa-mm-dd' : $aDatos['formatoFechas'];
		$this->aTraducciones = empty($aDatos['traducciones']) ? array() : $aDatos['traducciones'];

		return(true);
	}

	/**
	 * Devuelve el total de filas recuperadas, con y sin limit.
	 *
	 * @access public
	 * @param boolean $limit Reales (true) o sin limit (false, por defecto)
	 * @return integer
	 */
	public function totalFilas($limit = false){
		$iRet = 0;
		if($limit) $iRet = count($this->oCsv->aResultados);
		elseif(!empty($this->iTotalFilas)) $iRet = $this->iTotalFilas;

		return($iRet);
	}

	/**
	 * Actualiza una o mas filas
	 *
	 * @access public
	 * @param string $fichero Fichero en el que se hara la actualizacion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @param array $condicion Condicion de actualizacion, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function update($fichero, $datos, $condicion = array()){
		return false;
	}

	/**
	 * Condiciones de la consulta
	 * $campo puede ser un nombre de campo, o una condicion con indicaciones de los valores a comparar: "campo1 = %s AND campo2 IN (%u, %u)" (en cuyo caso $valor sera un array con los valores a sustituir)
	 *
	 * @access public
	 * @param string $campo Nombre del campo
	 * @param string $valor Valor o valores (array) con el que comparar el campo
	 * @return boolean
	 */
	public function where($campo, $valor = null){
		return false;
	}

}
