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
defined('CLASSES_DIR') or define('CLASSES_DIR', dirname(__FILE__));

/**
 * Interface para las clases de recuperacion de datos.
 * Capa de abstracción para recuperación de datos de distintas fuentes.
 *
 * @author José M. Carnero
 * @since 2015-03-04
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package RedAtazo
 */
interface iRedatazo {

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
	public function consulta($consulta = null, $parametros = array(), $numFilas = null, $numPagina = null);

	/**
	 * Borra una o mas filas
	 *
	 * @access public
	 * @param string $almacen Almacen de datos en el que se hara el borrado
	 * @param array $condicion Condicion de borrado, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function delete($almacen, $condicion = array());

	/**
	 * Almacen del que devolver resultados, tabla cuando SQL, fichero (cuando CSV, XML), etc.
	 *
	 * @access public
	 * @param string $almacen Almacen de datos (seguido de alias -opcional-)
	 * @return boolean
	 */
	public function from($almacen);

	/**
	 * Devuelve la consulta lanzada
	 * Completamente compuesta, tal como se haya enviado al motor de datos
	 *
	 * @access public
	 * @param string $posicion Que consulta mostrar, por defecto la ultima lanzada
	 * @return string
	 */
	public function getConsulta($posicion = 'ultima');

	/**
	 * Devuelve las filas recuperadas en la consulta
	 *
	 * @access public
	 * @return array
	 */
	public function getFilas();

	/**
	 * Inserta una fila
	 *
	 * @access public
	 * @param string $almacen Almacen de datos en el que se hara la insercion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function insert($almacen, $datos);

	/**
	 * Almacenes con los que hacer join del principal (definido con el metodo from)
	 *
	 * @access public
	 * @param string $almacen Almacen de datos (seguido de alias -opcional-), y seguido de condicion (si no se pone en el segundo parametro)
	 * @param string $condicion Condicion del join (-opcional-)
	 * @param string $tipo Tipo de join: inner, left, right
	 * @return boolean
	 */
	public function join($almacen, $condicion = null, $tipo = 'inner');

	/**
	 * Limita el numero de filas devuelto
	 *
	 * @access public
	 * @param integer $cantidad Cantidad de filas a devolver
	 * @param integer $offset Numero de fila desde la cual empezar a devolver
	 * @return boolean
	 */
	public function limit($cantidad, $offset = 0);

	/**
	 * Opciones de ordenacion
	 *
	 * @access public
	 * @param string $campo Campo por el que se ordenaran los resultados
	 * @param string $sentido Sentido de la ordenacion: asc(endente) o desc(endente)
	 * @return boolean
	 */
	public function order_by($campo, $sentido = 'asc');

	/**
	 * Campos a devolver
	 *
	 * @access public
	 * @param string $campos Campos a devolver, separados por comas
	 * @return boolean
	 */
	public function select($campos = '*');

	/**
	 * Actualiza una o mas filas
	 *
	 * @access public
	 * @param string $almacen Almacen de datos en el que se hara la actualizacion
	 * @param array $datos Datos a insertar, array asociativo ('campo' => valor)
	 * @param array $condicion Condicion de actualizacion, array asociativo ('campo' => valor)
	 * @return boolean
	 */
	public function update($almacen, $datos, $condicion = array());

	/**
	 * Condiciones de la consulta
	 * $campo puede ser un nombre de campo, o una condicion con indicaciones de los valores a comparar: "campo1 = %s AND campo2 IN (%u, %u)" (en cuyo caso $valor sera un array con los valores a sustituir)
	 *
	 * @access public
	 * @param string $campo Nombre del campo
	 * @param string $valor Valor o valores (array) con el que comparar el campo
	 * @return boolean
	 */
	public function where($campo, $valor = null);

}
