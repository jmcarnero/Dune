<?php
/*Copyright (C) 2009 José M. Carnero <jm_carnero@sargazos.net>

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

//dependencias
require_once CLASSES_DIR . '/interface.captchazo.inc';

/**
 * Clase base para creacion de CAPTCHA
 *
 * Tipos:
 * cuadro -> pinchar un cuadro dentro del captcha que se debe pinchar
 * letra -> sin imagen, pide escribir la palabra o letra que falta (se usa por defecto si no esta la libreria GD)
 * texto -> escribir el texto que aparezca
 *
 * Ejemplo de uso:
 * <code>
include_once("./class.captchazo.inc");

$oCaptcha = new Captchazo('texto');

$oCaptcha->iAncho = 100;
$oCaptcha->iAlto = 50;
$oCaptcha->aCaracteres = array('a','b','caracter','2+1'); //lo normal sera una lista de caracteres con los que construir el texto del CAPTCHA, pero se permite cualquier numero y combinacion de caracteres
//$oCaptcha->setDirectorio('/tmp/dir_captcha/');
//$oCaptcha->setBaseUrl('http://ejemplo.com/dir_captcha/');
//$oCaptcha->setDirectorio('base64'); //sin setBaseUrl() para salida en json y base64
$oCaptcha->generar();

$oCaptcha->getPregunta(); //texto informativo
$oCaptcha->getRespuesta(); //texto que ha de responder el usuario

//echo($oCaptcha->getHtml());
 * </code>
 *
 * @author José M. Carnero
 * @version 2b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Captchazo
 */
class Captchazo implements iCaptchazo {

	private $oCaptcha = null; //objeto Captcha con el que se va a generar

	/**
	 * Constructor
	 *
	 * @param string $sTipo Tipo de CAPTCHA que se devolvera
	 */
	function __construct($sTipo = 'letra'){
		$this->selTipo($sTipo);
	}

	/**
	 * Genera el Captcha
	 *
	 * @param integer $iTipo Tipo de pregunta
	 * @return boolean
	 */
	public function generar($iTipo = 0){
		return $this->oCaptcha->generar();
	}

	/**
	 * Devuelve HTML formado
	 *
	 * @return string
	 */
	public function getHtml(){
		return $this->oCaptcha->getHtml();
	}

	/**
	 * Devuelve la pregunta
	 *
	 * @return string
	 */
	public function getPregunta(){
		return $this->oCaptcha->sPregunta;
	}

	/**
	 * Devuelve la respuesta
	 *
	 * @return string
	 */
	public function getRespuesta(){
		return $this->oCaptcha->sRespuesta;
	}

	/**
	 * Selector de tipo de CAPTCHA.
	 *
	 * @throws Exception
	 * @param string $sTipo Tipo de CAPTCHA que se usara
	 * @return boolean
	 */
	private function selTipo($sTipo = 'letra'){
		$this->oCaptcha = null;
		$sLibreria = 'captchazo_' . $sTipo;
		$sClass = ucfirst($sLibreria);

		//si no esta instalada la libreria GD se fuerza a utilizar un captcha sin imagenes
		if(extension_loaded('gd') && function_exists('gd_info')){
			include_once CLASSES_DIR . '/class.' . $sLibreria . '.inc';
		}
		else{
			include_once CLASSES_DIR . '/class.captchazo_letra.inc';
		}

		if(!class_exists($sClass)){
			throw new Exception('No se puede usar el tipo de CAPTCHA [' . $sClass . ']');
			return false;
		}

		$this->oCaptcha = new $sClass();
		return true;
	}

	/**
	 * Asigna el juego de caracteres que se usara para generar el captcha (para los tipos de captcha basados en caracteres al azar)
	 *
	 * @param string $aCaracteres Juego de caracteres
	 * @return boolean
	 */
	public function setCaracteres($aCaracteres){
		return $this->oCaptcha->setCaracteres($aCaracteres);
	}

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sDirectorio Ruta del directorio
	 * @return boolean
	 */
	public function setDirectorio($sDirectorio){
		return $this->oCaptcha->setDirectorio($sDirectorio);
	}

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sBaseUrl URL base en la que se encuentra la imagen captcha
	 * @return boolean
	 */
	public function setBaseUrl($sBaseUrl){
		return $this->oCaptcha->setBaseUrl($sBaseUrl);
	}

}
