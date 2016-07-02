# Contributing to Project
All contributions (PRs) have to be done towards _develop_ branch. 
If a feature or bugfix is a major change please contact me to prepare feature specific handling.

__master__: Branch that contains latest production (stable) release. No PRs other than firmware releases will be merged into __master__.  
__develop__: Main development branch: contains latest features and fixes.

This will mean that __all contributors__ will have to submit a PR to _develop_ , it will be tested and then merged to __develop__ for automated integration testing (via TravisCI), as well as manual testing on a real device. 

__Project Contributing flow__:
- Fork repository
- Create a branch off  _develop_ (name it __feature/mynewfeature__ or __fix/myfix__)
- Build, test your code against a real device
- Commit changes
- Push your changes to your fork on github
- Submit PR to the main repo, __develop__ branch.
- Work with other contributors to test your feature and get it merged to _develop_

This is the most common approach for a git-flow:
http://nvie.com/posts/a-successful-git-branching-model/
