.PHONY: clean commit push

clean:
	@git rm . --cached -r

commit: 
	@git add .
	@git commit -m "update"

push: 
	@git push origin main
