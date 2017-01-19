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
require_once CLASSES_DIR . '/class.captchazo_funGraficas.inc';

/**
 * Creacion de CAPTCHA.
 * Con texto aleatorio, de diccionario o fijo.
 *
 * @author José M. Carnero
 */
class Captchazo_texto extends Captchazo_funGraficas implements iCaptchazo {

	/**
	 * @var string Stiqueta HTML con el CAPTCHA y lo que necesite para integrarlo directamente en la pagina, si es falso solo se envia el propio CAPTCHA (imagen o lo que corresponda)
	 */
	private $sHtml;

	public $aCaracteres; //lista de caracteres o palabras que se usaran //TODO permitir el uso de palabras de un diccionario
	public $iNumCaracteres; //numero de caracteres a dibujar

	/**
	 * Constructor
	 */
	public function __construct(){
		parent::__construct();

		$this->setCaracteres(array(
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', /*'Ñ', */'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', /*'ñ', */'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			1, 2, 3, 4, 5, 6, 7, 8, 9, 0,
			'!', '?', '@', '#', '+', '-', '*', '/', '=', '$', '%', '&'
		));
		$this->sPregunta = 'Escriba el texto que aparece en la imagen.';
		$this->iNumCaracteres = 6;
	}

	/**
	 * Crea el CAPTCHA
	 *
	 * El nombre de la imagen incluye el SHA1 de la respuesta, ej.: captchazo_076daa39afaaefb993769209a51e582d803a5cfb.png
	 * La fuente por defecto de las funciones gráficas sustituye la "ñ" y "Ñ" por a mayúscula con un acento circunflejo (al reves), haciendola inutilizable
	 *
	 * @param integer $iTipo Tipo de pregunta (sin uso)
	 * @return boolean
	 */
	public function generar($iTipo = 0){
		$iTamFuente = 5; //TODO usando las fuentes estandar de GD [5] es el tamano maximo

		//lista de caracteres que se usaran
		shuffle($this->aCaracteres);

		$this->sRespuesta = '';
		for($i = 0; $i < $this->iNumCaracteres; $i++){
			/*if(rand(0, 1)) $this->sRespuesta .= strtoupper($this->aCaracteres[rand(0, count($this->aCaracteres) - 1)]);
			else*/ $this->sRespuesta .= $this->aCaracteres[rand(0, count($this->aCaracteres) - 1)];
		}

		$this->sImagenNombre = 'captchazo_' . sha1($this->sRespuesta) . '.png';

		//se genera una imagen del tamaño del texto, luego se redimensionara al tamaño fijado por el usuario
		$iTempAncho = $this->iAncho;
		$iTempAlto = $this->iAlto;

		if(/*!is_readable($this->sDirectorio . $this->sImagenNombre) && */$this->creaImagen(imagefontwidth($iTamFuente) * strlen($this->sRespuesta) + 10, imagefontheight($iTamFuente) + 10)){
			$iColorTexto = imagecolorallocate($this->rImagen, $this->aColorTexto[0], $this->aColorTexto[1], $this->aColorTexto[2]);
			//TODO otras fuentes, ver "imageloadfont()" y "imagettftext()"
			imagestring($this->rImagen, $iTamFuente, 5, 5, $this->sRespuesta, $iColorTexto);

			if($this->iGDVersion >= 2){
				$rImagenAux = imagecreatetruecolor($iTempAncho, $iTempAlto);
				imagealphablending($rImagenAux, false); //desactivar procesamiento automatico de alpha
				imagesavealpha($rImagenAux, true); //alpha original guardado en destino
			}
			else{
				$rImagenAux = imagecreate($iTempAncho, $iTempAlto);
			}
			$iColorFondo = imagecolorallocate($rImagenAux, $this->aColorFondo[0], $this->aColorFondo[1], $this->aColorFondo[2]);

			if($this->iGDVersion >= 2){
				imagecopyresampled($rImagenAux, $this->rImagen, 0, 0, 0, 0, $iTempAncho, $iTempAlto, $this->iAncho, $this->iAlto);
			}
			else{
				imagecopyresized($rImagenAux, $this->rImagen, 0, 0, 0, 0, $iTempAncho, $iTempAlto, $this->iAncho, $this->iAlto);
			}

			imagedestroy($this->rImagen);

			//recupera ancho y alto originales
			$this->iAncho = $iTempAncho;
			$this->iAlto = $iTempAlto;

			$this->rImagen = &$rImagenAux;
			//imagedestroy($rImagenAux);

			$this->ruidoLineas(rand(15, 20));

			//$this->imagefilter(IMG_FILTER_GRAYSCALE);
			//$this->imagefilter(IMG_FILTER_COLORIZE, r, g, b, alpha);
			$this->imagefilter(IMG_FILTER_EDGEDETECT);
			//$this->imagefilter(IMG_FILTER_EMBOSS);
			//$this->imagefilter(IMG_FILTER_MEAN_REMOVAL);
			//$this->imagefilter(IMG_FILTER_SMOOTH, 5);

			$this->imagefilter(IMG_FILTER_NEGATE);
			$this->imagefilter(IMG_FILTER_GAUSSIAN_BLUR);

			//$gaussian = array(array(1.0, 2.0, 1.0), array(2.0, 4.0, 2.0), array(1.0, 2.0, 1.0));
			//$this->imageconvolution($gaussian, 16, 0);

			$this->guardaImagen();
		}

		if($this->sDirectorio !== false && $this->sDirectorio !== 'base64') $this->sHtml = <<<HTML
	<div clas="captchazo">
		<img src="{$this->sBaseUrl}{$this->sImagenNombre}" alt="CAPTCHA" /><br />
		<label class="captchazoPreg" alt="CAPTCHA" />{$this->sPregunta}</label>
		<input type="text" name="captchazo" value="" />
	</div>
HTML;

		return true;
	}

	/**
	 * Devuelve HTML formado
	 *
	 * @return string
	 */
	public function getHtml(){
		return $this->sHtml;
	}

	/**
	 * Asigna el juego de caracteres que se usara para generar el captcha (para los tipos de captcha basados en caracteres al azar)
	 *
	 * @param string $aCaracteres Juego de caracteres
	 * @return boolean
	 */
	public function setCaracteres($aCaracteres){
		if(!is_array($aCaracteres)){
			$aCaracteres = array($aCaracteres);
		}

		$this->aCaracteres = $aCaracteres;

		return true;
	}

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sDirectorio Ruta del directorio
	 * @return boolean
	 */
	public function setDirectorio($sDirectorio){
		if(!empty($sDirectorio)){
			if(strtolower(trim($sDirectorio)) == 'base64'){
				$this->sDirectorio = 'base64';
				return true;
			}
			elseif(!empty($sDirectorio) && is_writable($sDirectorio)){
				$this->sDirectorio = rtrim($sDirectorio, '/') . '/';
				return true;
			}
		}

		trigger_error('El directorio [' . $sDirectorio . '] no permite el guardado de las imagenes captcha generadas', E_USER_WARNING);
		return false;
	}

	/**
	 * Asigna el directorio en el que se guardara la imagen del captcha generado
	 *
	 * @param string $sBaseUrl URL base en la que se encuentra la imagen captcha
	 * @return boolean
	 */
	public function setBaseUrl($sBaseUrl){
		$this->sBaseUrl = rtrim($sBaseUrl, '/') . '/';
		return true;
	}

}
