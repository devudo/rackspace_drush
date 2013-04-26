<?php

/**
 * Implementation of hook_drush_command().
 */
function rackspace_drush_command() {
  $items['rackspace-dns-create'] = array(
    'description' => dt('Save a DNS record to Rackspace Cloud.'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'options' => array(
      'rackspace_username' => dt('Rackspace account username.'),
      'rackspace_tenant' => dt('Rackspace tenant name (can be anything)'),
      'rackspace_api_key' => dt('Rackspace API key'),
      'hostname' => dt('Full hostname to create a record.'),
      'ip_address' => dt('IP Address for the record.'),
    ),
  );
  return $items;
}

/**
 * Command function for rackspace-dns-create
 */
function drush_rackspace_dns_create(){
  
  // Include libraries
  require_once "php-opencloud/lib/php-opencloud.php";
  
  // Define our Rackspace options
  define('AUTHURL', RACKSPACE_US);
  define('USERNAME', drush_get_option('rackspace_username', ''));
  define('TENANT', drush_get_option('rackspace_tenant', ''));
  define('APIKEY', drush_get_option('rackspace_api_key', ''));

  $hostname = drush_get_option('hostname', '');
  $ip_address = drush_get_option('ip_address', '');
  
  // Process domain
  $domain_name = implode('.', array_splice(explode(".", $hostname), -2));
  
  // Check we have our options
  if (!USERNAME) {
    return drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('Rackspace Username required.'));
  }
  if (!$hostname) {
    return drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('Hostname is required.'));
  }
  if (!$ip_address) {
    return drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('IP Address is required.'));
  }
  
  $cloud = new \OpenCloud\Rackspace(AUTHURL, array(
    'username' => USERNAME,
    'apiKey' => APIKEY
  ));
  $dns = $cloud->DNS();

  drush_log(dt('Connected to Rackspace DNS API Service.'), 'ok');
  
 
  // Find our Domain Object 
  // Get a list of domains named "devudo.com" (there will only be one.)
  $dlist = $dns->DomainList(array('name' => $domain_name));
  while($domain = $dlist->Next()) {
    // Grab the domain ID
    if ($domain->Name() == $domain_name){
      $domain_id =   $domain->id;
    }
  }
  
  // If no domain ID, notify the user.
  if (!isset($domain_id)){
    return drush_set_error(DRUSH_FRAMEWORK_ERROR, dt('The parent domain %s was not found in your Rackspace account.  Please create it first.', array('%s' => $domain_name)));
  } else {
    drush_log(dt('Domain found. Adding A record for %s...', array('%s' => $domain_name)), 'ok');
  }
    
  // Create DNS record
  $domain = $dns->Domain($domain_id);
  $record = $domain->Record();
  $resp = $record->Create(array(
    'type' => 'A',
    'name' => $hostname,
    'ttl' => 600,
    'data' => $ip_address,
  ));
  return $resp->WaitFor("COMPLETED", 300, 'drush_rackspace_dns_create_wait', 1);
}

/**
 * callback for WaitFor method
 */
function drush_rackspace_dns_create_wait($obj) {
  if ($obj->Status() == 'ERROR') {
    $msg = dt('Error detected:');
    $msg .= sprintf("\n\tError code [%d] message [%s]\n\tDetails: %s\n", $obj->error->code, $obj->error->message, $obj->error->details);
		return drush_set_error($msg);
	}
	else if ($obj->Status() == 'COMPLETED') {
		drush_log(dt('Record added successfully!'), 'success');
	}
}