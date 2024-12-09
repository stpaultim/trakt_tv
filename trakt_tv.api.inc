<?php

/**
 * @file
 * Contains API-related functions for the Trakt TV module.
 */


/**
 * Fetch trending TV shows from Trakt API.
 */
function trakt_tv_fetch_trending() {
  $api_key = config_get('trakt_tv.settings', 'api_key');

  if (empty($api_key)) {
    watchdog('trakt_tv', 'API key not configured. Please set it in module settings.', [], WATCHDOG_ERROR);
    return [];
  }

  $api_url = 'https://api.trakt.tv/shows/trending';

  $options = [
    'http' => [
      'header' => [
        "Content-Type: application/json",
        "trakt-api-version: 2",
        "trakt-api-key: $api_key",
      ],
      'method' => 'GET',
    ],
  ];

  $context = stream_context_create($options);
  $response = @file_get_contents($api_url, false, $context);

  if ($response === FALSE) {
    $error = error_get_last();
    watchdog('trakt_tv', 'Failed to fetch trending shows: @message', [
      '@message' => $error['message'],
    ], WATCHDOG_ERROR);
    return [];
  }

  return json_decode($response, TRUE);
}


/**
 * Fetch TV show data based on search query.
 */
function trakt_tv_search_api($query) {
  $api_key = config_get('trakt_tv.settings', 'api_key');

  if (empty($api_key)) {
    watchdog('trakt_tv', 'API key not configured. Please set it in module settings.', [], WATCHDOG_ERROR);
    return [];
  }

  $query = trim(check_plain($query));
  $api_url = 'https://api.trakt.tv/search/show?query=' . urlencode($query);

  $options = [
    'http' => [
      'header' => [
        "Content-Type: application/json",
        "trakt-api-version: 2",
        "trakt-api-key: $api_key",
      ],
      'method' => 'GET',
    ],
  ];

  $context = stream_context_create($options);
  $response = @file_get_contents($api_url, false, $context);

  if ($response === FALSE) {
    watchdog('trakt_tv', 'Search API request failed for query: @query', [
      '@query' => $query,
    ], WATCHDOG_ERROR);
    return [];
  }

  return json_decode($response, TRUE);
}


/**
 * Fetch full TV show details by Trakt ID.
 */
function trakt_tv_fetch_show_details($trakt_id) {
  $api_key = config_get('trakt_tv.settings', 'api_key');

  if (empty($api_key)) {
    watchdog('trakt_tv', 'API key not configured. Please set it in module settings.', [], WATCHDOG_ERROR);
    return [];
  }

  $api_url = 'https://api.trakt.tv/shows/' . intval($trakt_id) . '?extended=full';

  $options = [
    'http' => [
      'header' => [
        "Content-Type: application/json",
        "trakt-api-version: 2",
        "trakt-api-key: $api_key",
      ],
      'method' => 'GET',
    ],
  ];

  $context = stream_context_create($options);
  $response = @file_get_contents($api_url, false, $context);

  if ($response === FALSE) {
    watchdog('trakt_tv', 'Failed to fetch show details for Trakt ID: @id', [
      '@id' => $trakt_id,
    ], WATCHDOG_ERROR);
    return [];
  }

  return json_decode($response, TRUE);
}

