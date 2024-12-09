# Trakt TV API Module

This module has been created to integrate with the Trakt TV API in a limited
manner. This is a new module, custom made for Backdrop CMS.

It currently, does the following. 

1) Allows a site vistor to view a list of the top 10 trending shows on Trakt TV.
2) Allows a site vistor to search for a specific TV show in the Trakt TV database.
3) Allows a site vistor to create a node with the following data for any TV show.
   - Title
   - Trakt ID
   - Desciption / Overview
   - Rating
   - Tagline
   - Year
   - Status
   - Website
   - Genres
   - Network
  
This module creates a content type called TV Show with all of the fields required by
this module. 

## Requirements

- This module requires a Trakt TV API Key. You will need to create an account on
  Trakt TV and create an application for an APP API key.
  https://trakt.tv/oauth/applications/new

## Installation

- Install this module using the official [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

- You will need to create an API key at Trakt TV and add it to the Trakt TV configuration
  page. Your key will be stored in the site config files. Please, be cautious. We are hoping
  to find a better solution in the near future (see OpenAI module solution). 

## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/openai/issues).

## Current Maintainer

- [Tim Erickson](https://github.com/stpaultim)

## Credits

- Sponsored by Simplo

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
