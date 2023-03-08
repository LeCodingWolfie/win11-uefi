pandoc md/docs.md --verbose --standalone -o html/docs.html
pandoc md/re.md --verbose --standalone -o html/re.html --highlight-style md/themes/github-dark.theme --template md/default.html5
