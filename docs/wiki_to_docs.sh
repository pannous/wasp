# brew install pandoc
# cd docs 
for path in ../wiki/*.md; do
		file=${path##*/}
		echo "$file"
    pandoc "$path" -o "out/${file%.md}.html"
done

cd out
for file in *.html; do
		echo "$file"
		sed -i '' 's|\[\[\([^]]*\)\]\]|<a href="\1.html">\1</a>|g' "$file"
done