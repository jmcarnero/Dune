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
 * Interface para las clases de recuperacion de datos.
 * Capa de abstracción para recuperación de datos de distintas fuentes.
 *
 * @author José M. Carnero
 * @since 2015-10-15
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Captchazo
 */
interface iCaptchazo {

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

}
