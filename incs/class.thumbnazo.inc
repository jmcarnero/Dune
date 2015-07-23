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
 * Generacion de thumbnails (de momento libreria GD)
 *
 * $sImagen -> ruta de la imagen original
 *
 * Ejemplo de uso:
 * \code{.php}
$oThumbnail = new Thumbnazo('../img/imagen.jpg');
$oThumbnail->crear();
 * \endcode
 *
 * @author José M. Carnero
 * @version 1.0b
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Thumbnazo
 */
class Thumbnazo {

	private $iAltura; //altura del thumb en pixels o en porcentaje (ej. 50% sin espacios); por defecto 100px
	private $iAnchura; //anchura del thumb en pixels o en porcentaje (ej. 50% sin espacios); por defecto 100px
	private $sExtension; //extension del thumb, por defecto la misma de la imagen original
	private $aLibreria; //array con los datos de la libreria: lib (nombre de la libreria: GD, ImageMagick, ...), version, soporte de jpg, gif, etc.
	private $sImagen; //nombre imagen original (incluido path)
	private $sNombre; //nombre de thumbnail
	private $sPrefijo; //prefijo del thumb
	private $sRuta; //ruta donde guardar thumbs; si se omite se intenta guardar en la misma de $sImagen
	private $sSufijo; //sufijo del thumb
	private $sTipoEscalado; //tipo de escalado de la imagen: 'proporcional' (por defecto, encajara la imagen en el cuadro pasado como ancho y alto, sin deformarla), 'fijo' (ajustara la imagen al ancho y alto pasado, deformandola si no corresponde)
	
	//constructor
	function __construct($sImagen = '', $iAncho = 100, $iAlto = 100){
		$this->libreria();
		$this->setSufijos();
		$this->setRutas($sImagen);
		$this->setDimensiones($iAncho, $iAlto);

		$this->sExtension = null;
		$this->sTipoEscalado = 'proporcional';
	}

	/**
	 * Creacion del thumbnail, la ruta y nombre del archivo original se deben pasar antes (bien a la propiedad o a los distintos metodos
	 *
	 * @todo comprobar tamaño de la imagen original y verificar si hay espacio en memoria para completar el proceso, llamar solo a este procedimiento para todo el proceso (sin incluir dar valores a propiedades)
	 */
	public function crear(){
		$this->crearRuta();
		$this->crearNombre();

		$aImgOriginalDatos = $this->datosImgOriginal(); //obtiene los datos de la imagen original

		switch($aImgOriginalDatos['tipo']){
			case 'jpeg':
			case 'jpg':
				$dImg = imagecreatefromjpeg($this->sImagen);
				break;
			case 'gif':
				$dImg = imagecreatefromgif($this->sImagen);
				break;
			case 'png':
				$dImg = imagecreatefrompng($this->sImagen);
				break;
			default:
				trigger_error('El formato de imagen [' . $aImgOriginalDatos['tipo'] . '] no está soportado', E_USER_ERROR);
				return false; //El formato de imagen solicitado no esta soportado actualmente
	}

		switch($this->aLibreria['version']){
			case '1':
				$dThumb = imagecreate($this->iAnchura, $this->iAltura); //para GD 1 o 2 (mas rapida y ligera para el servidor, menos calidad)
				break;
			case '2':
				$dThumb = imagecreatetruecolor($this->iAnchura, $this->iAltura); //para GD 2
				break;
			default:
				trigger_error('Libreria ' . $this->aLibreria['lib'] . '[' . $this->aLibreria['version'] . '] no permite crear imagen', E_USER_ERROR);
				return false; //Version de libreria no soportada
		}

		//reescalar
		switch($this->aLibreria['version']){
			case '1':
				imagecopyresized($dThumb, $dImg, 0, 0, 0, 0, $this->iAnchura, $this->iAltura, $aImgOriginalDatos['ancho'], $aImgOriginalDatos['alto']); //para GD 1 o 2 (mas rapida y ligera para el servidor, menos calidad)
				break;
			case '2':
				imagecopyresampled($dThumb, $dImg, 0, 0, 0, 0, $this->iAnchura, $this->iAltura, $aImgOriginalDatos['ancho'], $aImgOriginalDatos['alto']); //para GD 2
				break;
			default:
				trigger_error('Libreria ' . $this->aLibreria['lib'] . '[' . $this->aLibreria['version'] . '] no permite reescalado', E_USER_ERROR);
				return false; //Version de libreria no soportada
		}

		//guardar imagen
		switch($this->sExtension){
			case 'jpeg':
			case 'jpg':
				imagejpeg($dThumb, $this->sRuta . $this->sNombre, 75); //TODO de momento el nivel de calidad no es personalizable
				break;
			case 'gif':
				imagegif($dThumb, $this->sRuta . $this->sNombre);
				break;
			case 'png':
				imagepng($dThumb, $this->sRuta . $this->sNombre);
				break;
			default:
				trigger_error('Libreria ' . $this->aLibreria['lib'] . '[' . $this->aLibreria['version'] . '] no permite guardar imagen', E_USER_ERROR);
				return false; //no se ha podido guardar
		}

		imagedestroy($dImg);
		return true;
	}

	/**
	 * Crear nombre del thumbnail
	 */
	private function crearNombre(){
		$iCont = 0;

		$aNombreImagen = explode('.', basename($this->sImagen));

		$sNombre = $aNombreImagen[0]; //nombre de la imagen original, sin extension

		$this->sExtension = empty($this->sExtension) ? $aNombreImagen[1] : $this->sExtension;

		$sNombreFichero = $this->sPrefijo . $sNombre . $this->sSufijo . '.' . $this->sExtension;

		if(file_exists($this->sRuta . $sNombreFichero)){
			while(($iCont++) < 1000){
				$sNombreFichero = $this->sSufijo . $sNombre . $this->sSufijo . '_' . $iCont . '.' . $this->sExtension;

				if(!file_exists($this->sRuta . $sNombreFichero)){ //nombre unico de fichero //TODO pisar anterior?
					break;
				}
			}
		}

		$this->sNombre = $sNombreFichero;
	}

	/**
	 * Crear directorio si no existe, sin parametros iguala el directorio del thumb al de la imagen original
	 *
	 * @return boolean
	 */
	private function crearRuta(){
		if(empty($this->sRuta)){
			$aTmp = explode('/', $this->sImagen);
			array_pop($aTmp); //nombre de la imagen original, sin ruta
			$this->sRuta = implode('/', $aTmp).'/'; //ruta de la imagen original, sin nombre
		}

		if(!is_writable($this->sRuta) && !mkdir($this->sRuta, 0775)){
			trigger_error('No se puede crea ruta [' . $this->sRuta . '] o no se puede escribir en ella', E_USER_ERROR);
			return false; //No se ha podido crear el directorio
		}

		return true;
	}

	/**
	 * Fijar tamaño del thumb y extraer datos de la imagen original
	 *
	 * @todo dimensiones en porcentaje
	 * @return array Datos de la imagen original
	 */
	private function datosImgOriginal(){
		$aDatos = array();

		//tamaño de la imagen original
		list($aDatos['ancho'], $aDatos['alto'], $aDatos['tipo']) = getimagesize($this->sImagen);
		$aDatos['tipo'] = strtolower(substr($this->sImagen, strrpos($this->sImagen, '.') + 1)); //extrae extension y la convierte a minusculas

		//escalado de la imagen
		//TODO calculo cuando se pasen ancho y alto en porcentaje
		switch($this->sTipoEscalado){
			case 'fijo':
				break;
			default: //'proporcional'
				//proporcion
				$x = $aDatos['ancho']/$this->iAnchura;
				$y = $aDatos['alto']/$this->iAltura;
				$escala = 0;

				//escala
				if($x > $y) $escala = $this->iAnchura/$aDatos['ancho'];
				if($x < $y) $escala = $this->iAltura/$aDatos['alto'];
				if($x == $y){
					if($this->iAnchura > $this->iAltura) $escala = $this->iAltura/$aDatos['alto'];
					if($this->iAltura > $this->iAnchura) $escala = $this->iAnchura/$aDatos['ancho'];
				}

				//Escala la imagen
				$this->iAnchura = intval($aDatos['ancho']*$escala);
				$this->iAltura = intval($aDatos['alto']*$escala);
		}

		return $aDatos;
	}

	/**
	 * Version de la libreria GD (si esta)
	 *
'GD Version' => string '2.0.36' (length=6)
'FreeType Support' => boolean true
'FreeType Linkage' => string 'with freetype' (length=13)
'T1Lib Support' => boolean false
'GIF Read Support' => boolean true
'GIF Create Support' => boolean true
'JPEG Support' => boolean true
'PNG Support' => boolean true
'WBMP Support' => boolean true
'XPM Support' => boolean false
'XBM Support' => boolean false
'JIS-mapped Japanese Font Support' => boolean false
	 */
	private function libreria(){
		//obtener version de libreria GD (pone propiedad a falso si no hay libreria, condicionales con ===)
		if(extension_loaded('gd')){
			$aGDinfo = gd_info();
			//$this->aLibreria['version'] = substr(ereg_replace('[[:alpha:][:space:]()]+', '', $aGDinfo['GD Version']), 0, 1);

			$this->aLibreria = array(
				'lib' => 'GD',
				'version' => substr(preg_replace('/[[:alpha:][:space:]()]+/im', '', $aGDinfo['GD Version']), 0, 1),
				'gif' => $aGDinfo['GIF Create Support'],
				'jpg' => $aGDinfo['JPEG Support'],
				'png' => $aGDinfo['PNG Support']
			);
		}
		else{
			$this->aLibreria = array(
				'lib' => false,
				'version' => false,
				'gif' => false,
				'jpg' => false,
				'png' => false
			);
		}
	}

	/**
	 * Establece las dimensiones del thumbnail
	 *
	 * @param integer $iAncho Ancho del thumbnail en pixels
	 * @param integer $iAlto Alto del thumbnail en pixels
	 */
	public function setDimensiones($iAncho = 100, $iAlto = 100){
		$iAncho = (int) $iAncho;
		$iAlto = (int) $iAlto;

		if(!empty($iAncho))
			$this->iAnchura = $iAncho;

		if(!empty($iAlto))
			$this->iAltura = $iAlto;
			}

	/**
	 * Asigna la ruta de la imagen y donde guardar el thumbnail
	 * Si solo se pasa la ruta de la imagen se intentara guardar el thumbnail en la misma ruta
	 *
	 * @param string $sImagen Imagen original, ruta y nombre
	 * @param string $sRuta Ruta donde guardar el thumbnail
	 */
	public function setRutas($sImagen = '', $sRuta = null){
		$this->sImagen = $sImagen;
		$this->sRuta = $sRuta;
			}

	/**
	 * Asigna prefijo y sufijo que se añadiran al nombre de la imagen original para crear el nombre del thumbnail
	 * Si se omiten ambos se sustituye la imagen original (en caso de tener la misma extension)
	 */
	public function setSufijos($sPrefijo = '', $sSufijo = '_thb'){
		$this->sPrefijo = empty($sPrefijo) ? '' : $sPrefijo;
		$this->sSufijo = empty($sSufijo) ? '' : $sSufijo;
			}

}
