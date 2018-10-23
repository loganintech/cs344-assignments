build:
	cargo build --release

install: build move

move:
	mv target/release/buildrooms .
	mv target/release/adventure .


clean:
	rm buildrooms
	rm adventure
	rm -rf sasol.rooms.*
