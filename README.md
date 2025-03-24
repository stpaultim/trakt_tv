# Trakt TV API Module

This module has been created to integrate with the [Trakt TV](https://trakt.tv/) API in a limited
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

![image](https://github.com/user-attachments/assets/655b077f-beb0-407e-bae4-51668a62e908)

We are open to idea for how to expand this module.


## Requirements

- This module requires a Trakt TV API Key. You will need to create an account on
  Trakt TV and create an application for an APP API key.
  https://trakt.tv/oauth/applications/new

## Installation

- Install this module using the official [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

- You will need to create an account with Trakt TV and create an app here to the necessary credentials for this module. https://trakt.tv/oauth/applications

These fields are required.
![image](https://github.com/user-attachments/assets/5a64fcef-0a63-498b-bf14-709f2b676fdf)

Copy the Client ID, Client Secret, and Redirect URI. For the final step, you will need to "authorize" the URI and paste the provided code into `admin/config/media/trakt_tv/auth-exchange`.

![image](https://github.com/user-attachments/assets/5f2cd152-3065-49b9-bd1e-4ebab55ef7f2)

Your code will be here after you authorize your site:
![image](https://github.com/user-attachments/assets/b9c6308f-669e-4d14-a4d2-0cf5beaf2c8d)


## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/openai/issues).

## Current Maintainer

- [Tim Erickson](https://github.com/stpaultim)

## Credits

- Sponsored by Simplo

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
