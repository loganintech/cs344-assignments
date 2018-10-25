install: build move

build:
	cargo build --release


move:
	mv target/release/buildrooms .
	mv target/release/adventure .


clean:
	rm buildrooms
	rm adventure
	rm -rf sasol.rooms.*
