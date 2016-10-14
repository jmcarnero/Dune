<?php if(!defined('DUNE')) die('...error');

$aMod['error'] = array('title' => $this->trad('Incidencia'));
$sTitle = $aMod['error']['title'];
$sMetas = '';
$sCss = '';
$sJs = '';
?><div class="no_encontrado">
<?php if(defined('CIERRE_APP') && CIERRE_APP){ ?><p><?php echo $this->trad('Disculpa'); ?> ...<br /><?php echo $this->trad('hemos cerrado temporalmente'); ?>.</p>
<?php }elseif(isset($_GET['c'])){ ?><p><?php echo $this->trad('Tu sesión ha caducado'); ?> ...<br /><?php echo($this->trad('Recuerda que después de ').floor(ini_get('session.gc_maxlifetime') / 60).' '.$this->trad('minutos de').'<br />'.$this->trad('inactividad será cerrada por seguridad')); ?>.</p>
<?php }elseif(isset($_GET['d'])){ ?><p><?php echo $this->trad('Has intentado acceder'); ?> ...<br /><?php echo $this->trad('a una sección no permitida'); ?>.</p>
<?php }else{ ?><p><?php echo $this->trad('Disculpa'); ?> ...<br /><?php echo $this->trad('no hemos encontrado la página que buscas'); ?>.</p><?php } ?>
</div>
