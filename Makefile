.PHONY: commit push

commit:
	@git add .
	@git commit -m "update"

push: commit
	@git push origin main
