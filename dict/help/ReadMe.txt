After update help/ru/ru.po
cd help/ru/
git rm -r html stardict.xml
cd help/
make stardict-html
cd help/ru/
git add html stardict.xml

git commit -m "Update help/ru/"
git push

