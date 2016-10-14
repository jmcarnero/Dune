<?php
/*Copyright (C) 2016 José M. Carnero <jm_carnero@sargazos.net>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
http://www.gnu.org/copyleft/gpl.html*/

/**
 * Interface para las clases SQLazo
 *
 * @author José M. Carnero
 * @since 2016-10-11
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package SQLazo
 */
interface iSQLazo {

	/**
	 * Lanza una consulta y devuelve sus resultados
	 *
	 * @param string $sConsulta Consulta a preparar
	 * @param array $aParametros Parametros para la consulta, en el mismo orden que aparezcan en la SQL; se sustituira cada ? por el parametro en la posicion correspondiente
	 * @return void
	 */
	public function consulta($sConsulta, $aParametros = array());

	/**
	 * Devuelve el numero de filas afectadas por la ultima consulta
	 *
	 * Para consultas SELECT devuelve el numero de filas recuperadas (el forzado con LIMIT si fuera el caso)
	 *
	 * @return integer
	 */
	public function totalFilas();

	/**
	 * Muestra la ultima consulta lanzada y sus parametros
	 *
	 * @param boolean $bCruda Si true devuelve la consulta y sus parametros tal como hayan sido proporcionados a la clase; si false devuelve la consulta lanzada por la base de datos (si no se ha producido error que lo impida)
	 * return array
	 */
	public function ultimaConsulta($bCruda = false);

	/**
	 * Devuelve el ultimo Id (en consultas INSERT con clave primaria autonumerica) insertado
	 *
	 * @return integer
	 */
	public function ultimoId();

}

