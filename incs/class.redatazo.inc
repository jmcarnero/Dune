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
 * Capa de abstracción para recuperación de datos de distintas fuentes.
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package RedAtazo
 */

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta.
defined('CLASSES_DIR') or define('CLASSES_DIR',dirname(__FILE__));
include_once (CLASSES_DIR . '/interface.redatazo.inc');

/**
 * Clase base de recuperacion de datos.
 *
 * @since 2015-03-01
 */
class Redatazo implements iRedatazo {

	/**
	 * Parametros con los que construir la consulta:
	 * 'motor' => ['csv'|'mysql'|'xml']
	 * 'server' => ['localhost'|ip]
	 * 'database' => ['db']
	 * 'usuario' => ['user']
	 * 'clave' => ['pass']
	 *
	 * @see Redatazo::setParametros($parametros)
	 * @var array
	 * @access private
	 */
	protected $aParametros = array();

	function __construct($parametros = array()){
		$this->setParametros($parametros);
	}

	/**
	 * Intenta conectar e agregar la clase de datos indicada en los parametros
	 */
	private function conecta(){
		switch($this->aParametros['motor']){
			case 'csv':
				if(is_readable(CLASSES_DIR . '/class.redatazo_csv.inc')){
					include_once (CLASSES_DIR . '/class.redatazo_csv.inc');
					$this->oRedatazo = new Redatazo_csv($this->aParametros);
				}
				else
					throw new Exception('Fichero de Motor [redatazo_csv] no disponible');
				break;
			case 'mysql':
			case 'oracle':
				if(is_readable(CLASSES_DIR . '/class.redatazo_ddbb.inc')){
					include_once (CLASSES_DIR . '/class.redatazo_ddbb.inc');
					$this->oRedatazo = new Redatazo_ddbb($this->aParametros);
				}
				else
					throw new Exception('Fihcero de Motor [redatazo_ddbb] no disponible');
				break;
			case 'xml':
				throw new InvalidArgumentException('Motor no implementado');
				break;
			default:
				throw new InvalidArgumentException('Motor desconocido');
		}

		return true;
	}

	/**
	 * Lanza la consulta.
	 *
	 * @access public
	 * @param string $consulta Consulta a lanzar para recuperar datos; puede omitirse si se ha construido con select/from/...
	 * @param array $parametros Parametros para la consulta, si no se pasa la consulta en el primer parametro, tal como los requiera el motor de datos concreto
	 * @param integer $numFilas Numero de filas a devolver
	 * @param integer $numPagina Numero de pagina en la que empezar a devolver filas
	 * @return array
	 */
	public function consulta($consulta = null, $parametros = array(), $numFilas = null, $numPagina = null){
		return $this->oRedatazo->consulta($consulta, $parametros, $numFilas, $numPagina);
	}

	/**
	 * Borra una o mas filas
	 *
	 * @access public
	 * @param string $almacen Almacen de datos en el que se hara el borrado
	 * @param array $condicion Condicion de borrado, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function delete($almacen, $condicion = array()){
		return $this->oRedatazo->delete($almacen, $condicion);
	}

	/**
	 * Almacen del que devolver resultados, tabla cuando SQL, fichero (cuando CSV, XML), etc.
	 *
	 * @access public
	 * @param string $almacen Almacen de datos (seguido de alias -opcional-)
	 * @return boolean
	 */
	public function from($almacen){
		return $this->oRedatazo->from($almacen);
	}

	/**
	 * Devuelve la consulta lanzada
	 * Completamente compuesta, tal como se haya enviado al motor de datos
	 *
	 * @access public
	 * @param string $posicion Que consulta mostrar, por defecto la ultima lanzada
	 * @return string
	 */
	public function getConsulta($posicion = 'ultima'){
		return $this->oRedatazo->getConsulta($posicion);
	}

	/**
	 * Devuelve las filas recuperadas en la consulta
	 *
	 * @access public
	 * @return array
	 */
	public function getFilas(){
		return $this->oRedatazo->getFilas();
	}

	/**
	 * Inserta una fila
	 *
	 * @access public
	 * @param string $almacen Almacen de datos en el que se hara la insercion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function insert($almacen, $datos){
		return $this->oRedatazo->insert($almacen, $datos);
	}

	/**
	 * Almacenes con los que hacer join del principal (definido con el metodo from)
	 *
	 * @access public
	 * @param string $almacen Almacen de datos (seguido de alias -opcional-), y seguido de condicion (si no se pone en el segundo parametro)
	 * @param string $condicion Condicion del join (-opcional-)
	 * @param string $tipo Tipo de join: inner, left, right
	 * @return boolean
	 */
	public function join($almacen, $condicion = null, $tipo = 'inner'){
		return $this->oRedatazo->join($almacen, $condicion, $tipo);
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
		return $this->oRedatazo->limit($cantidad, $offset);
	}

	/**
	 * Opciones de ordenacion
	 *
	 * @access public
	 * @param string $campo Campo por el que se ordenaran los resultados; sin orden por defecto
	 * @param string $sentido Sentido de la ordenacion: asc(endente) o desc(endente)
	 * @return boolean
	 */
	public function order_by($campo, $sentido = 'asc'){
		return $this->oRedatazo->order_by($campo, $sentido);
	}

	/**
	 * Campos a devolver
	 *
	 * @access public
	 * @param string $campos Campos a devolver, separados por comas
	 * @return boolean
	 */
	public function select($campos = '*'){
		return $this->oRedatazo->select($campos);
	}

	/**
	 * Prepara los parametros que se utilizaran en la recuperacion de datos.
	 * Recoge todos los posibles parametros de las distintas variantes de acceso a datos: SQL, CSV, XML, ...
	 *
	 * @access public
	 * @param array $parametros Asociativo con los datos requeridos por la clase
	 * @return boolean
	 */
	public function setParametros($parametros = array()){
		if(empty($parametros['motor'])){
			trigger_error('Motor de datos obligatorio', E_USER_ERROR);
			return false;
		}

		$this->aParametros['motor'] = $parametros['motor'];

		//sql
		$this->aParametros['server'] = empty($parametros['server']) ? 'localhost' : $parametros['server'];
		$this->aParametros['database'] = empty($parametros['database']) ? 'test' : $parametros['database'];
		$this->aParametros['usuario'] = empty($parametros['usuario']) ? 'usuario' : $parametros['usuario'];
		$this->aParametros['clave'] = empty($parametros['clave']) ? 'clave' : $parametros['clave'];

		//csv
		$this->aParametros['filaTitulos'] = empty($parametros['filaTitulos']) ? 0 : $parametros['filaTitulos'];
		$this->aParametros['separadorCampos'] = empty($parametros['separadorCampos']) ? ',' : $parametros['separadorCampos'];

		$this->conecta();
	}

	/**
	 * Devuelve el total de filas recuperadas, sin limit.
	 *
	 * @access public
	 * @return integer
	 */
	public function totalFilas(){
		return $this->oRedatazo->totalFilas();
	}

	/**
	 * Actualiza una o mas filas
	 *
	 * @access public
	 * @param string $almacen Almacen de datos en el que se hara la actualizacion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @param array $condicion Condicion de actualizacion, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function update($almacen, $datos, $condicion = array()){
		return $this->oRedatazo->update($almacen, $datos, $condicion);
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
		return $this->oRedatazo->where($campo, $valor);
	}

}
