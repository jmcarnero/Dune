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

/**
 * Capa de abstracción para recuperación de datos de distintas fuentes con una única interfaz.
 *
 * @author José M. Carnero
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package REdAtazo
 */

//Constante con el directorio donde se encuentran las clases derivadas y/o dependientes de esta
defined('CLASSES_DIR') or define('CLASSES_DIR', dirname(__FILE__));
include_once (CLASSES_DIR . '/interface.redatazo.inc');
include_once (CLASSES_DIR . '/class.sqlazo.inc');

/**
 * Clase para recuperar datos de base de datos.
 * Depende de "class.sqlazo.inc".
 *
 * @since 2015-03-01
 */
class Redatazo_ddbb implements iRedatazo {

	//almacen de fragmentos de la consulta para la posterior composicion
	protected $aConsulta = array('select' => array(), 'from' => '', 'join' => array(), 'where' => array(), 'order_by' => array());

	/**
	 * Objeto de base de datos (class.sqlazo)
	 *
	 * @var object
	 */
	private $oDDBB;

	/**
	 * Base de datos a usar para las consultas
	 *
	 * @var string
	 */
	private $sDDBB;

	/**
	 * Clave del usuario de la base de datos
	 *
	 * @var string
	 */
	private $sDDBBPassword;

	/**
	 * Servidor de base de datos
	 *
	 * @var string
	 */
	private $sDDBBServidor;

	/**
	 * Motor de base de datos (mysql, postgresql)
	 *
	 * @var string
	 */
	private $sDDBBTipo;

	/**
	 * Usuario con el que conectar a la base de datos
	 *
	 * @var string
	 */
	private $sDDBBUsuario;

	//filas recuperadas en la consulta
	private $aFilas = array();

	//numero de filas a devolver, false si no se limita
	private $iNumeroFilas;

	//numero de fila en la que empezar a devolver
	private $iOffsetFilas;

	/**
	 * Prefijo de las tablas
	 *
	 * @var string
	 */
	private $sPrefijo;

	/**
	 * Usuario que lanza las consultas (id del usuario)
	 *
	 * @var string
	 */
	private $sTrazabilidad;

	/**
	 * Parametros para la SQL
	 *
	 * @var array
	 */
	private $aSql;

	/**
	 * SQL enviada por el usuario o compuesta desde los "fragmentos" enviados por el usuario con cada metodo
	 *
	 * @var string
	 */
	private $sSql;

	/**
	 * Ultima consulta lanzada; tal como la haya enviado el usuario o como se haya construido (sin parametros)
	 *
	 * @var string
	 * @see rdatazo_ddbb::getConsulta()
	 */
	private $sUltimaSql = '';

	/**
	 * Constructor
	 */
	function __construct($aDatos = array()){
		$this->setParametros($aDatos);

		$this->iTotalFilas = 0;
		$this->iNumeroFilas = false;
		$this->iOffsetFilas = 0;

		$this->conectar();
	}

	function __destruct(){
		$this->oDDBB->desconectar();
	}

	/**
	 * Conectar con la base de datos
	 *
	 * @todo eliminar esta conexion y hacer una nueva si cambian parametros, $this->setParametros
	 * @return boolean
	 */
	private function conectar(){
		//conexion a base de datos
		//TODO control de errores
		$this->oDDBB = sqlazo_sel($this->sDDBBTipo);
		if(!$this->oDDBB->conectar($this->sDDBBServidor, $this->sDDBBUsuario, $this->sDDBBPassword, $this->sDDBB)){
			trigger_error('No se puede conectar al servidor: ' . $this->sDDBBServidor, E_USER_ERROR);
			return false;
		}
		$this->oDDBB->sPrefijo = $this->sPrefijo;
		if(!empty($this->sTrazabilidad)){
			$this->oDDBB->bTrazabilidad = true;
			$this->oDDBB->sIdUsuario = $this->sTrazabilidad;
		}

		return true;
	}

	/**
	 * Ejecucion de consulta
	 *
	 * @todo pasar $numFilas y $numPagina a $this->limit; teniendo en cuenta no pisar si no se pasa nada
	 * @param string $consulta Consulta a lanzar para recuperar datos; puede omitirse si se ha construido con select/from/...
	 * @param array $parametros Parametros para la consulta, si no se pasa la consulta en el primer parametro, tal como los requiera el motor de datos concreto
	 * @param integer $numFilas Numero de filas a devolver
	 * @param integer $numPagina Numero de pagina en la que empezar a devolver filas
	 * @return boolean
	 */
	public function consulta($consulta = null, $parametros = array(), $numFilas = null, $numPagina = null){
		$this->setConsulta($consulta, $parametros);

		if(!empty($numFilas)) $this->iNumeroFilas = $numFilas;
		if(!empty($numPagina)) $this->iOffsetFilas = ($numFilas * $numPagina);

		if($this->oDDBB->consulta($this->sSql, $this->aSql, $this->iNumeroFilas, $this->iOffsetFilas)){
			$this->sSql = $this->oDDBB->sQuery; //consulta lanzada
		}
		else{
			trigger_error('No se puede ejecutar la consulta: [' . $this->sSql . ']', E_USER_WARNING);
			return false; //error
		}

		/*if($this->oDDBB->leeFilasTodos()){
			$this->iTotalFilas = $this->oDDBB->iTotalFilas;
			$this->aFilas = $this->oDDBB->aFilas;
			$aReturn = true;
		}
		else{
			$aReturn = null; //error
		}*/

		$this->sUltimaSql = $this->sSql;

		$this->limpiaConsulta(); //despues de lanzar una consulta debe limpiarse para empezar de cero la siguiente
		return true;
	}

	/**
	 * Borra una o mas filas
	 *
	 * @param string $tabla Tabla en la que se hara el borrado
	 * @param array $condicion Condicion de borrado, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function delete($tabla, $condicion = array()){
		$this->sSql = 'DELETE FROM ' . $tabla;
		$this->aSql = array();

		if(!empty($condicion) && is_array($condicion)){
			foreach($condicion as $clave => $valor){
				$this->where($clave, $valor);
			}
		}
		if(!empty($this->aConsulta['where'])){
			$this->sSql .= ' WHERE ' . implode(' AND ', $this->aConsulta['where']) . "\n";
			$this->aSql = array_merge($this->aSql, $this->aConsulta['where_p']);

			if($this->consulta($this->sSql, $this->aSql))
				return true;
		}
		else{
			trigger_error('No se pueden hacer borrados sin WHERE', E_USER_WARNING);
		}

		return false;
	}

	/**
	 * Tabla de la que devolver resultados
	 * "from" no se limpia, se interpreta como nombre de tabla (o parte de nombre) valido
	 *
	 * @todo poner nombre de tabla y alias entre ` (tildes graves)
	 * @param string $tabla Tabla (seguida de alias -opcional-)
	 * @return boolean
	 */
	public function from($tabla){
		$tabla = trim($tabla);
		if(empty($tabla))
			return false;

		$this->aConsulta['from'] = $tabla;
	}

	/**
	 * Devuelve la consulta lanzada
	 * Completamente compuesta, tal como se haya enviado al motor de datos
	 *
	 * @param string $posicion Que consulta mostrar, por defecto la ultima lanzada
	 * @return string
	 */
	public function getConsulta($posicion = 'ultima'){
		$sConsulta = $this->oDDBB->sQuery;
		if(empty($sConsulta))
			$sConsulta = '[ERROR] ' . $this->sUltimaSql; //si hay algun tipo de error devuelve la consulta que se ha enviado al controlador de datos

		return $sConsulta;
	}

	/**
	 * Devuelve las filas recuperadas en la consulta
	 *
	 * @return integer Id insertada
	 */
	public function getFilas(){
		if($this->oDDBB->leeFilasTodos())
			return $this->oDDBB->aFilaTodos;

		return false;
	}

	/**
	 * Inserta una fila
	 *
	 * @param string $tabla Tabla en la que se hara la insercion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @return integer Id insertada o null (cuando error)
	 */
	public function insert($tabla, $datos){
		$this->sSql = 'INSERT INTO ' . $tabla;
		$this->aSql = array();

		$aPreVal = $aPostVal = array();
		foreach($datos as $clave => $valor){
			$aPreVal[] = $clave;
			$aPostVal[] = "'%s'";
			$this->aSql[] = $valor;
		}

		$this->sSql .= ' (' . implode(', ', $aPreVal) . ') VALUES (' . implode(', ', $aPostVal) . ')';

		if($this->consulta($this->sSql, $this->aSql))
			return $this->oDDBB->sUltimaId;

		return null;
	}

	/**
	 * Devuelve la ultima id producida en una insercion (last_insert_id)
	 *
	 * @return integer Id insertada o null
	 */
	public function insert_id(){
		if(!empty($this->oDDBB->sUltimaId))
			return $this->oDDBB->sUltimaId;

		return null;
	}

	/**
	 * Tablas con las que hacer join de la principal
	 *
	 * @todo poner nombre de tabla y alias entre ` (tildes graves); decidir si el usuario puede enviar la palabra clave "join" y tipo de join en $tabla
	 * @param string $tabla Tabla (seguida de alias -opcional-), y seguida de condicion (si no se pone en el segundo parametro)
	 * @param string $condicion Condicion del join (-opcional-)
	 * @param string $tipo Tipo de join: inner, left, right
	 * @return boolean
	 */
	public function join($tabla, $condicion = null, $tipo = 'inner'){
		if(strtolower($tipo) == 'left')
		 $tipo = ' LEFT JOIN ';
		elseif(strtolower($tipo) == 'right')
		$tipo = ' RIGHT JOIN ';
		else
			$tipo = ' INNER JOIN ';

		if(empty($condicion))
			$this->aConsulta['join'][] = $tipo . $tabla;
		else
			$this->aConsulta['join'][] = $tipo . $tabla . ' ON ' . $condicion;
	}

	/**
	 * Limita el numero de filas devuelto
	 *
	 * @param integer $cantidad Cantidad de filas a devolver
	 * @param integer $offset Numero de fila desde la cual empezar a devolver
	 * @return boolean
	 */
	public function limit($cantidad, $offset = 0){
		$this->iNumeroFilas = $cantidad;
		$this->iOffsetFilas = $offset;
		return true;
	}

	private function limpiaConsulta(){
		$this->aConsulta = array('select' => array(), 'from' => '', 'join' => array(), 'where' => array(), 'order_by' => array());
		$this->sSql = '';
		$this->aSql = array();
	}

	/**
	 * Opciones de ordenacion
	 *
	 * @todo poner nombre de tabla y campos entre ` (tildes graves); decidir si el usuario puede enviar las palabras clave "order by"
	 * @param string $campo Campo por el que se ordenaran los resultados; sin orden por defecto
	 * @param string $sentido Sentido de la ordenacion: asc(endente) o desc(endente)
	 * @return boolean
	 */
	public function order_by($campo, $sentido = 'asc'){
		$sentido = (empty($sentido) || $sentido != 'desc' ? ' ASC' : ' DESC');
		 $this->aConsulta['order_by'][] = $campo . $sentido;
	}

	/**
	 * Campos a devolver
	 *
	 * @todo poner nombre de campos entre ` (tildes graves); decidir si el usuario puede enviar la palabra clave "select"
	 * @param string $campos Campos a devolver, separados por comas
	 * @return boolean
	 */
	public function select($campos = '*'){
		$this->aConsulta['select'][] = $campos;
	}

	/**
	 * Construye la consulta
	 *
	 * @param string $consulta Consulta completa si no se envia fragmentada a cada metodo
	 * @param array $parametros Parametros para la consulta, si no se pasa la consulta en el primer parametro, tal como los requiera el motor de datos concreto
	 * @return array
	 */
	private function setConsulta($consulta = null, $parametros = array()){
		$this->sSql = '';
		$this->aSql = array();

		if(empty($consulta)){
			if(!empty($this->aConsulta['select'])) $this->sSql .= 'SELECT ' . implode(', ', $this->aConsulta['select']) . "\n";

			if(!empty($this->aConsulta['from'])) $this->sSql .= ' FROM ' . $this->aConsulta['from'] . "\n";

			if(!empty($this->aConsulta['join'])) $this->sSql .= implode("\n", $this->aConsulta['join']) . "\n";

			if(!empty($this->aConsulta['where'])){
				$this->sSql .= ' WHERE ' . implode(' AND ', $this->aConsulta['where']) . "\n";
				if(!empty($this->aConsulta['where_p'])){
					$this->aSql += $this->aConsulta['where_p'];
				}
			}

			if(!empty($this->aConsulta['order_by'])) $this->sSql .= ' ORDER BY ' . implode(', ', $this->aConsulta['order_by']) . "\n";
		}
		else{
			$this->sSql = $consulta;
			$this->aSql = $parametros;
		}
	}

	/**
	 * Prepara los parametros que se utilizaran en la recuperacion de datos.
	 * $aDatos:
	 * 'tipo' => tipo de base de datos (mysql por defecto)
	 * 'servidor' => servidor de base de datos (localhost por defecto)
	 * 'usuario' => usuario para conectar a la base de datos (test por defecto)
	 * 'clave' => clave para conectar a la base de datos (nada por defecto)
	 * 'esquema' => base de datos (test por defecto)
	 * 'prefijotablas' => prefijo que llevaran las tablas de la base de datos (ninguno por defecto)
	 * 'trazabilidad' => usuario que lanza las consultas, si no esta vacio se trazaran las consultas con este usuario (vacio por defecto)
	 *
	 * @param array $aDatos Asociativo con los datos requeridos por la clase
	 * @return boolean
	 */
	public function setParametros($aDatos = array()){
		if(empty($aDatos)) return(false);

		$this->sDDBBTipo = empty($aDatos['motor']) ? 'mysql' : $aDatos['motor'];
		$this->sDDBBServidor = empty($aDatos['server']) ? 'localhost' : $aDatos['server'];
		$this->sDDBBUsuario = empty($aDatos['usuario']) ? 'nadie' : $aDatos['usuario'];
		$this->sDDBBPassword = empty($aDatos['clave']) ? 'clave' : $aDatos['clave'];
		$this->sDDBB = empty($aDatos['database']) ? 'test' : $aDatos['database'];
		$this->sPrefijo = empty($aDatos['prefijotablas']) ? '' : $aDatos['prefijotablas'];
		$this->sTrazabilidad = empty($aDatos['trazabilidad']) ? false : $aDatos['trazabilidad'];

		return true;
	}

	/**
	 * Devuelve el total de filas recuperadas, sin limit.
	 *
	 * @return integer
	 */
	public function totalFilas(){
		return $this->oDDBB->iTotalFilas;
	}

	/**
	 * Actualiza una o mas filas
	 *
	 * @todo generar una advertencia si no hay where
	 * @param string $tabla Tabla en la que se hara la actualizacion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @param array $condicion Condicion de actualizacion, array asociativo ('campo' => valor); tambien se pueden asignar en el metodo where
	 * @return boolean
	 */
	public function update($tabla, $datos, $condicion = array()){
		$this->sSql = 'UPDATE ' . $tabla . ' SET';
		$this->aSql = array();

		$aVals = array();
		foreach($datos as $clave => $valor){
			$aVals[] = "$clave = '%s'";
			$this->aSql[] = $valor;
		}
		$this->sSql .= ' ' . implode(', ', $aVals);

		if(!empty($condicion) && is_array($condicion)){
			foreach($condicion as $clave => $valor){
				$this->where($clave, $valor);
			}
		}
		if(!empty($this->aConsulta['where'])){
			$this->sSql .= ' WHERE ' . implode(' AND ', $this->aConsulta['where']) . "\n";
			$this->aSql = array_merge($this->aSql, $this->aConsulta['where_p']);
		}
		else{
			trigger_error('Consulta UPDATE sin WHERE', E_USER_NOTICE);
		}

		if($this->consulta($this->sSql, $this->aSql))
			return true;

		return false;
	}

	/**
	 * Condiciones de la consulta
	 * $campo puede ser un nombre de campo, o una condicion con indicaciones de los valores a comparar: "campo1 = %s AND campo2 IN (%u, %u)" (en cuyo caso $valor sera un array con los valores a sustituir)
	 *
	 * @todo poner nombre de campos entre ` (tildes graves); decidir si el usuario puede enviar la palabra clave "where"
	 * @param string $campo Nombre del campo
	 * @param string $valor Valor con el que comparar el campo
	 * @return boolean
	 */
	public function where($campo, $valor = null){
		$sAux = '';
		if(!empty($valor)){
			if(is_null($valor)){
				$this->aConsulta['where_p'] = array();
			}
			elseif(is_numeric($valor)){
				$sAux = " = %u";
				$this->aConsulta['where_p'][] = $valor;
			}
			elseif(strtolower($valor) == 'null'){
				$sAux = " = null";
			}
			else{
				$sAux = " = '%s'";
				$this->aConsulta['where_p'][] = $valor;
			}
		}
		$this->aConsulta['where'][] = $campo . $sAux;
	}

}
