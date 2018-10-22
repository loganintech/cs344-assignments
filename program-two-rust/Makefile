both: rooms-bin adventure-bin

rooms-bin:
	cd ./rooms && \
	cargo build --release && \
	mv target/release/buildrooms ..


adventure-bin:
	cd ./adventure && \
	cargo build --release && \
	mv target/release/adventure_game ..

clean:
	rm buildrooms
	rm adventure_game
	rm -rf sasol.rooms.*
