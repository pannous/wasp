increase-package-version.py wasp.js 
git -c color.ui=false commit -a -m '' --allow-empty-message
git push
ssh pannous.com "cd wasp && git pull"