<?php 

function trakt_tv_auth_code_exchange_form($form, &$form_state) {
  // Token status
  $tokens = trakt_tv_token_store_get();

  if ($tokens && isset($tokens['access_token'])) {
    $created = format_date($tokens['created_at'], 'short');
    $expires_at = $tokens['created_at'] + $tokens['expires_in'];
    $expires = format_date($expires_at, 'short');
    $remaining = $expires_at - time();

    $form['token_status'] = [
      '#type' => 'markup',
      '#markup' => '<div class="messages status"><strong>Access Token Info:</strong><br>' .
        'Created: ' . check_plain($created) . '<br>' .
        'Expires: ' . check_plain($expires) . '<br>' .
        'Time remaining: ' . ($remaining > 0 ? format_interval($remaining) : 'Expired') .
        '</div>',
    ];
  }
  else {
    $form['token_status'] = [
      '#type' => 'markup',
      '#markup' => '<div class="messages warning">No access token stored yet.</div>',
    ];
  }

  // Auth code input
  $form['auth_code'] = [
    '#type' => 'textfield',
    '#title' => t('Authorization Code'),
    '#description' => t('Paste the code you received from Trakt after approving the app.'),
    '#required' => TRUE,
    '#size' => 80,
  ];

  $form['submit'] = [
    '#type' => 'submit',
    '#value' => t('Exchange Code for Token'),
  ];

  return $form;
}

function trakt_tv_auth_code_exchange_form_submit($form, &$form_state) {
  $code = trim($form_state['values']['auth_code']);
  dpm("code");
  dpm($code);
  $success = trakt_tv_exchange_auth_code($code);

  if ($success) {
    backdrop_set_message(t('Access token successfully retrieved and saved.'));
  }
  else {
    backdrop_set_message(t('Failed to retrieve access token. Check the logs for details.'), 'error');
  }

  // Rebuild the form so updated token info is shown
  $form_state['rebuild'] = TRUE;
}

