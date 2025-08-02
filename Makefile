MAIN=src/otp
GENK=src/genkey

all:
	$(CC) src/main.c src/otpfile.c -o $(MAIN)
	$(CC) src/genkey.c -o $(GENK)


clean:
	rm -rf $(MAIN)
	rm -rf $(GENK)

define checkifroot
	if [ `id -u` -ne 0 ]; then \
		echo 'Need root privileges!'; \
		exit 1; \
	fi
endef

install:
	@$(call checkifroot)
	@if [ ! -e "$(MAIN)" ] && [ ! -e "$(GENK)" ]; then \
		echo "Try 'make' first."; \
		exit 1; \
	fi;

	cp src/otp /usr/bin/; cp src/genkey /usr/bin/; \
	chmod +x /usr/bin/otp; chmod +x /usr/bin/genkey; \

uninstall:
	@$(call checkifroot)
	rm /usr/bin/otp /usr/bin/genkey
