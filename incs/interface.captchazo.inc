<?php
/*Copyright (C) 2015 José M. Carnero <jm_carnero@sargazos.net>

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
	 * Genera el Captcha
	 *
	 * @param integer $iTipo Tipo de pregunta
	 * @return boolean
	 */
	public function generar($iTipo = 0);

	/**
	 * Devuelve HTML formado
	 *
	 * @return string
	 */
	public function getHtml();

	/**
	 * Asigna el juego de caracteres que se usara para generar el captcha (para los tipos de captcha basados en caracteres al azar)
	 *
	 * @param string $aCaracteres Juego de caracteres
	 * @return boolean
	 */
	public function setCaracteres($aCaracteres);

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sDirectorio Ruta del directorio
	 * @return boolean
	 */
	public function setDirectorio($sDirectorio);

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sBaseUrl URL base en la que se encuentra la imagen captcha
	 * @return boolean
	 */
	public function setBaseUrl($sBaseUrl);

}
