<?php

/**
 * Implementation of hook_drush_command().
 */
function rackspace_drush_command() {
  $items['rackspace-dns-create'] = array(
    'description' => dt('Save a DNS record to Rackspace Cloud.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'arguments' => array(
      'hostname' => dt('Full hostname to create a record.'),
      'ip' => dt('IP Address for the record.'),
    ),
  );
  return $items;
}