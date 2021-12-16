.PHONY: commit push

clean:
	@git rm . --cached -r

commit: 
	@git add .
	@git commit -m "update"

push: commit
	@git push origin main
