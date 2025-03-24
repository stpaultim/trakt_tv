<?php

/**
 * @file
 * Contains API-related functions for the Trakt TV module.
 */

function trakt_tv_token_store_get() {
  $file = 'private://trakt_tv_tokens.json';

  if (!file_exists($file)) {
    watchdog('trakt_tv', 'Token file not found at @path.', ['@path' => $file], WATCHDOG_WARNING);
    return NULL;
  }

  $json = file_get_contents($file);
  $data = json_decode($json, true);

  if (json_last_error() !== JSON_ERROR_NONE) {
    watchdog('trakt_tv', 'Invalid token JSON: @error', ['@error' => json_last_error_msg()], WATCHDOG_ERROR);
    return NULL;
  }

  return $data;
}

function trakt_tv_token_store_save(array $tokens) {
  $path = 'private://trakt_tv_tokens.json';
  $json = json_encode($tokens, JSON_PRETTY_PRINT);

  if (file_unmanaged_save_data($json, $path, FILE_EXISTS_REPLACE)) {
    watchdog('trakt_tv', 'Access token and refresh token saved to file.', [], WATCHDOG_INFO);
    return TRUE;
  }
  else {
    watchdog('trakt_tv', 'Failed to write token data to @path.', ['@path' => $path], WATCHDOG_ERROR);
    return FALSE;
  }
}

/**
 * Fetch trending TV shows from Trakt API.
 */
function trakt_tv_fetch_trending() {
  $access_token = trakt_tv_get_valid_token();
  $creds = trakt_tv_get_credentials();

  if (!$creds) {
    backdrop_set_message(t('You have not entered API credentials for Trakt TV yet. Please <a href="@url">go to the settings page</a> to configure them.', [
      '@url' => url('admin/config/media/trakt_tv'),
    ]), 'info');
  }

  if (!$access_token) {
    watchdog('trakt_tv', '002 - Missing Trakt access token.', [], WATCHDOG_ERROR);
    return [];
  }

  $ch = curl_init('https://api.trakt.tv/shows/trending');
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_HTTPHEADER, [
    "Accept: application/json",
    "Content-Type: application/json",
    "trakt-api-version: 2",
    "trakt-api-key: " . $creds['client_id'],
    "Authorization: Bearer $access_token",
  ]);

  // Set browser-like User-Agent to try bypassing Cloudflare bot detection
  curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36');

  $response = curl_exec($ch);
  $error = curl_error($ch);
  $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  curl_close($ch);

  if ($response === false) {
    watchdog('trakt_tv', 'cURL failed: @error', ['@error' => $error], WATCHDOG_ERROR);
    return [];
  }

  if ($status !== 200) {
    watchdog('trakt_tv', 'Trakt API returned status: @status - @body', [
      '@status' => $status,
      '@body' => $response,
    ], WATCHDOG_ERROR);
    return [];
  }

  $data = json_decode($response, TRUE);
  if (json_last_error() !== JSON_ERROR_NONE) {
    watchdog('trakt_tv', 'JSON decode error: @error', [
      '@error' => json_last_error_msg(),
    ], WATCHDOG_ERROR);
    return [];
  }

  return $data;
}


function trakt_tv_get_valid_token() {
  $tokens = trakt_tv_token_store_get();
  $creds = trakt_tv_get_credentials();

  if (!$tokens || !isset($tokens['access_token'])) {
    watchdog('trakt_tv', '001 - Missing Trakt token data.', [], WATCHDOG_ERROR);
    return NULL;
  }

  $expires_at = $tokens['created_at'] + $tokens['expires_in'];
  if (time() < $expires_at - 60) {
    return $tokens['access_token']; // still valid
  }

  // Refresh token
  $payload = [
    'refresh_token' => $tokens['refresh_token'],
    'client_id' => $creds['client_id'],
    'client_secret' => $creds['client_secret'],
    'redirect_uri' => $creds['redirect_uri'],
    'grant_type' => 'refresh_token',
  ];

  $opts = [
    'http' => [
      'header' => "Content-type: application/json",
      'method' => 'POST',
      'content' => json_encode($payload),
    ]
  ];

  $context = stream_context_create($opts);
  $response = file_get_contents('https://api.trakt.tv/oauth/token', false, $context);

  if ($response === FALSE) {
    watchdog('trakt_tv', 'Failed to refresh Trakt token.', [], WATCHDOG_ERROR);
    return NULL;
  }

  $data = json_decode($response, true);

  trakt_tv_token_store_save([
    'access_token' => $data['access_token'],
    'refresh_token' => $data['refresh_token'],
    'expires_in' => $data['expires_in'],
    'created_at' => time(),
  ]);

  return $data['access_token'];
}



/**
 * Fetch TV show data based on search query.
 */
function trakt_tv_search_api($query) {
  $access_token = trakt_tv_get_valid_token();
  $creds = trakt_tv_get_credentials();

  if (!$access_token) {
    watchdog('trakt_tv', 'Unable to fetch valid Trakt access token.', [], WATCHDOG_ERROR);
    backdrop_set_message(t('Unable to authenticate with Trakt. Please check token settings.'), 'error');
    return [];
  }

  $query = trim(check_plain($query));
  $api_url = 'https://api.trakt.tv/search/show?query=' . urlencode($query);

  $ch = curl_init($api_url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_HTTPHEADER, [
    "Accept: application/json",
    "Content-Type: application/json",
    "trakt-api-version: 2",
    "trakt-api-key: " . $creds['client_id'],
    "Authorization: Bearer $access_token",
  ]);
  curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64)');

  $response = curl_exec($ch);
  $error = curl_error($ch);
  $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  curl_close($ch);

  if ($response === false) {
    watchdog('trakt_tv', 'Search API cURL failed: @error', ['@error' => $error], WATCHDOG_ERROR);
    return [];
  }

  if ($status !== 200) {
    watchdog('trakt_tv', 'Search API returned status: @status - @body', [
      '@status' => $status,
      '@body' => $response,
    ], WATCHDOG_ERROR);
    return [];
  }

  $data = json_decode($response, TRUE);
  if (json_last_error() !== JSON_ERROR_NONE) {
    watchdog('trakt_tv', 'Search JSON decode error: @error', [
      '@error' => json_last_error_msg(),
    ], WATCHDOG_ERROR);
    return [];
  }

  return $data;
}



/**
 * Fetch full TV show details by Trakt ID.
 */
function trakt_tv_fetch_show_details($trakt_id) {
  $access_token = trakt_tv_get_valid_token();
  $creds = trakt_tv_get_credentials();

  if (!$access_token) {
    watchdog('trakt_tv', 'Unable to fetch valid Trakt access token.', [], WATCHDOG_ERROR);
    return [];
  }

  $api_url = 'https://api.trakt.tv/shows/' . intval($trakt_id) . '?extended=full';

  $ch = curl_init($api_url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  curl_setopt($ch, CURLOPT_HTTPHEADER, [
    "Accept: application/json",
    "Content-Type: application/json",
    "trakt-api-version: 2",
    "trakt-api-key: " . $creds['client_id'],
    "Authorization: Bearer $access_token",
  ]);
  curl_setopt($ch, CURLOPT_USERAGENT, 'Mozilla/5.0 (Windows NT 10.0; Win64; x64)');

  $response = curl_exec($ch);
  $error = curl_error($ch);
  $status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
  curl_close($ch);

  if ($response === false) {
    watchdog('trakt_tv', 'Show details cURL failed: @error', ['@error' => $error], WATCHDOG_ERROR);
    return [];
  }

  if ($status !== 200) {
    watchdog('trakt_tv', 'Show details API returned status: @status - @body', [
      '@status' => $status,
      '@body' => $response,
    ], WATCHDOG_ERROR);
    return [];
  }

  $data = json_decode($response, TRUE);
  if (json_last_error() !== JSON_ERROR_NONE) {
    watchdog('trakt_tv', 'Show details JSON decode error: @error', [
      '@error' => json_last_error_msg(),
    ], WATCHDOG_ERROR);
    return [];
  }

  return $data;
}

function trakt_tv_exchange_auth_code($code) {
  $creds = trakt_tv_get_credentials(); // Load from your private:// JSON

  if (!$creds) {
    watchdog('trakt_tv', 'Unable to access credentials.', [], WATCHDOG_ERROR);
    return [];
  }

  $data = [
    'code' => $code,
    'client_id' => $creds['client_id'],
    'client_secret' => $creds['client_secret'],
    'redirect_uri' => $creds['redirect_uri'],
    'grant_type' => 'authorization_code',
  ];

  $options = [
    'method' => 'POST',
    'data' => json_encode($data),
    'headers' => [
      'Content-Type' => 'application/json',
    ],
  ];

  $response = backdrop_http_request('https://api.trakt.tv/oauth/token', $options);

  if ($response->code != 200) {
    watchdog('trakt_tv', 'Token exchange failed: @data', ['@data' => $response->data], WATCHDOG_ERROR);
    return FALSE;
  }

  $token_data = json_decode($response->data, TRUE);

  // Save tokens to private file
  trakt_tv_token_store_save([
    'access_token' => $token_data['access_token'],
    'refresh_token' => $token_data['refresh_token'],
    'expires_in' => $token_data['expires_in'],
    'created_at' => time(),
  ]);

  backdrop_set_message(t('Trakt access token successfully saved.'));
  return TRUE;
}

function trakt_tv_get_credentials() {
  $path = 'private://credentials.json';
  
  if (file_exists($path)) {
    $json = file_get_contents($path);
    $data = json_decode($json, TRUE);
    
    if (json_last_error() === JSON_ERROR_NONE) {
      return $data;
    }
    else {
      watchdog('trakt_tv', 'Invalid JSON in credentials file: @error', [
        '@error' => json_last_error_msg(),
      ], WATCHDOG_ERROR);
    }
  }
  else {
    watchdog('trakt_tv', 'Credentials file not found at @path.', ['@path' => $path], WATCHDOG_ERROR);
  }

  return [];
}




