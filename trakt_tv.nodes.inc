<?php

/**
 * @file
 * Contains node-related functions for the Trakt TV module.
 */


/**
 * Creates a node from a Trakt TV ID.
 */
function trakt_tv_create_node_from_trakt_id($trakt_id) {
  $show = trakt_tv_fetch_show_details($trakt_id);

  if (empty($show) || !isset($show['title'])) {
    backdrop_set_message(t('Failed to fetch or invalid show data.'), 'error');
    return '<p>No data found for this TV show.</p>';
  }

  // Check if the show already exists.
  if (trakt_tv_show_exists($show['ids']['trakt'])) {
    backdrop_set_message(t('A node for this TV show already exists.'), 'warning');
    return '<p>A node for "' . check_plain($show['title']) . '" already exists.</p>';
  }

  // Create the new node.
  $node = new Node([
    'type' => 'tv_show', // Replace with your content type's machine name.
  ]);

  $node->title = $show['title'] ?? 'Unknown Title';
  $node->body = [
    'und' => [
      [
        'value' => $show['overview'] ?? 'No description available.',
        'format' => 'filtered_html',
      ],
    ],
  ];
  $node->field_rating = [
    'und' => [
      [
        'value' => $show['rating'] ?? 'None',
      ],
    ],
  ];
  $node->field_tagline = [
    'und' => [
      [
        'value' => $show['tagline'] ?? '',
      ],
    ],
  ];
  $node->field_year = [
    'und' => [
      [
        'value' => $show['year'] ?? 'Unknown',
      ],
    ],
  ];
  $node->field_status = [
    'und' => [
      [
        'value' => $show['status'] ?? '',
      ],
    ],
  ];
  $node->field_website = [
    'und' => [
      [
        'url' => $show['homepage'] ?? '',
      ],
    ],
  ];
  $node->field_certification = [
    'und' => [
      [
        'value' => $show['certification'] ?? 'None',
      ],
    ],
  ];
  $node->field_trakt_id = [
    'und' => [
      [
        'value' => $trakt_id,
      ],
    ],
  ];
  $genres = isset($show['genres']) && is_array($show['genres']) ? $show['genres'] : ['Unknown'];

  // Creates an array of all new taxonomy terms

  $new_terms = [];

  // Check if $genres is an array.
  if (isset($genres) && is_array($genres)) {
    foreach ($genres as $term_name) {
      // Check if the term already exists in the 'genres' vocabulary.
      $existing_terms = taxonomy_get_term_by_name($term_name, 'genres'); // Replace 'genres' with your vocabulary machine name.
      
      // If the term does not exist, add it to the $new_terms array.
      if (empty($existing_terms)) {
        if (!in_array($term_name, $new_terms)) {
          $new_terms[] = $term_name;
        }
      }
    }
  }

  // Add all new terms to the 'genres' vocabulary.
  foreach ($new_terms as $new_term_name) {
    $term = entity_create('taxonomy_term', [
      'name' => $new_term_name,
      'vocabulary' => 'genres',
    ]);
    taxonomy_term_save($term);
  }

  // Find tid for each new term and add term to node
  $term_key = 0;
  foreach ($genres as $genre) {
    $temp_tag = taxonomy_term_load_multiple_by_name($genre, 'genres');
    if (!empty($temp_tag)) {
      $term_tid = array_keys($temp_tag);
      $node->field_genres['und'][$term_key++]['tid'] = $term_tid[0];
    }
  }

  // Adding network term.
  $network = trim($show['network']); // Trim spaces to avoid duplicate terms.

  // Check if the term exists in the 'network' vocabulary.
  $existing_terms = taxonomy_get_term_by_name($network, 'network');

  if (!empty($existing_terms)) {
    // Term already exists, use its TID.
    $term2 = reset($existing_terms); // Get the first matching term.
    watchdog('trakt_tv', 'Found existing taxonomy term: @term', ['@term' => $network], WATCHDOG_DEBUG);
  } else {
    // Term does not exist, create it.
    $term2 = entity_create('taxonomy_term', [
      'name' => $network,
      'vocabulary' => 'network',
    ]);

    // Save the term only if it was successfully created.
    if ($term2) {
      taxonomy_term_save($term2);
      watchdog('trakt_tv', 'Created new taxonomy term: @term', ['@term' => $network], WATCHDOG_NOTICE);
    } else {
      watchdog('trakt_tv', 'Failed to create taxonomy term: @term', ['@term' => $network], WATCHDOG_ERROR);
    }
  }

  // Assign the TID to the node.
  if (!empty($term2->tid)) {
    $node->field_network['und'][0]['tid'] = $term2->tid;
  }
  $node->status = 1; // Published.
  $node->promote = 1; // Promoted to the front page.
  $node->language = LANGUAGE_NONE;

  // Set default values for comment-related fields.
  $node->uid = 1; // Default author (user ID 1 is the admin).
  $node->last_comment_uid = 1; // Default to admin as the last commenter.
  $node->last_comment_timestamp = REQUEST_TIME; // Use the current time.
  $node->comment = 0; // Disable comments unless explicitly enabled.

  $node->save();


  watchdog('trakt_tv', 'Created node for TV show: @title', ['@title' => $node->title], WATCHDOG_NOTICE);
  backdrop_set_message(t('TV show "@title" has been successfully created.', ['@title' => $node->title]));
  backdrop_goto('node/' . $node->nid);
}


/**
 * Check to see if node already exists and display appropriate links.
 *
 * @param array $show
 *   The TV show data from the Trakt API.
 *
 * @return string
 *   HTML output indicating the status of the node.
 */
function trakt_tv_node_check($show) {
  $title = $show['show']['title'];
  $year = $show['show']['year'];
  $trakt_id = $show['show']['ids']['trakt'];

  if (trakt_tv_show_exists($trakt_id)) {
    // Get the nid of the existing node.
    $nid = db_query("
      SELECT n.nid 
      FROM {node} n
      INNER JOIN {field_data_field_year} y ON n.nid = y.entity_id
      WHERE n.title = :title 
        AND y.field_year_value = :year
        AND n.type = :type",
      [
        ':title' => $title,
        ':year' => $year,
        ':type' => 'tv_show',
      ]
    )->fetchField();

    return '<em>Already exists</em> - <a href="/node/' . $nid . '">(view)</a>';
  } else {
    return '<a href="/trakt-tv-create-node-from-id/' . $show['show']['ids']['trakt'] . '">Add to database</a>';
  }
}


/**
 * Check if a TV show already exists based on Trakt ID.
 *
 * @param int $trakt_id
 *   The Trakt ID of the TV show.
 *
 * @return bool
 *   TRUE if the show exists, FALSE otherwise.
 */
function trakt_tv_show_exists($trakt_id) {
  // Query the database to check if a node exists with the same Trakt ID.
  $nid = db_query("
    SELECT n.nid 
    FROM {node} n
    INNER JOIN {field_data_field_trakt_id} t ON n.nid = t.entity_id
    WHERE t.field_trakt_id_value = :trakt_id
      AND n.type = :type",
    [
      ':trakt_id' => $trakt_id,
      ':type' => 'tv_show', // Replace with your content type machine name.
    ]
  )->fetchField();

  // Return TRUE if an existing node ID is found, otherwise FALSE.
  return !empty($nid);
}
