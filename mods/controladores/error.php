<?php
# Copyright (C) 2014 José M. Carnero <jm_carnero@sargazos.net>
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

//namespace Dune;
if(!defined('DUNE')) die('...');

/**
 * Controlador de errores
 *
 * @author José M. Carnero
 * @since 2014-11-26
 * @version 1
 * @license http://www.gnu.org/copyleft/gpl.html
 * @package Dune
 */
class Error extends Controlazo {

	function __construct(){
		parent::__construct(__FILE__);
		$this->sSeccion = strtolower(__CLASS__);

		$this->aDatos['aMod'][$this->sSeccion] = array('title' => $this->trad('Error'));

		$this->aDatos['sMetas'] = '<meta name="keywords" content="Sargazos.net">';
		$this->aDatos['sTitle'] = $this->aDatos['aMod'][$this->sSeccion]['title'];
		$this->aDatos['sCss'] .= '';
		$this->aDatos['sJs'] .= '';

		$this->tipoError();
		$this->pinta('dune', $this->sSeccion);
	}

	/**
	 * Tipo de mensaje de error
	 *
	 * Espera recibir codigos de error HTTP
	 */
	protected function tipoError(){
		$sTipo = empty($_GET['tipo']) ? 404 : $_GET['tipo']; //generico y seccion no encontrada

		$this->aDatos['sMensaje'] = $this->trad('Disculpa'). ' ...<br />' . $this->trad('no encuentro la página que buscas');

		if(defined('CIERRE_APP') && CIERRE_APP){
			$sTipo = '503';
		}

		switch($sTipo){
			case '401': //Unauthorized
			case '403': //Forbidden access
				$this->aDatos['sMensaje'] = $this->trad('Has intentado acceder') . ' ...<br />' . $this->trad('a una sección no permitida');
				break;
			case '408': //Request Timeout; en este caso se refiere a sesion caducada
				$this->aDatos['sMensaje'] = $this->trad('Tu sesión ha caducado') . ' ...<br />' . $this->trad('Recuerda que después de ') . floor(ini_get('session.gc_maxlifetime') / 60) . ' ' . $this->trad('minutos de') . '<br />' . $this->trad('inactividad será cerrada por seguridad');
				break;
			case '503': //Service Unavailable
				$this->aDatos['sMensaje'] = $this->trad('Disculpa') . ' ...<br />' . $this->trad('hemos cerrado temporalmente');
				break;
			case '404': //Not found
			default:
		}

		$this->aDatos['sMensaje'] .= '.';
	}

}
