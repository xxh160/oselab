.PHONY: commit push

clean:
	@git rm . --cached -r

commit: clean
	@git add .
	@git commit -m "update"

push: commit
	@git push origin main
