# Makefile for FNA ABI Compatibility Files
# Written by Ethan "flibitijibibo" Lee

ASSEMBLIES = \
	Microsoft.Xna.Framework \
	Microsoft.Xna.Framework.Game \
	Microsoft.Xna.Framework.Graphics \
	Microsoft.Xna.Framework.GamerServices \
	Microsoft.Xna.Framework.Input.Touch \
	Microsoft.Xna.Framework.Net \
	Microsoft.Xna.Framework.Storage \
	Microsoft.Xna.Framework.Video \
	Microsoft.Xna.Framework.Xact

TARGETS = $(ASSEMBLIES:%=%.dll)

%.dll: %.cs FNA.dll FNA.NetStub.dll
	mkdir -p bin
	mcs -delaysign+ -keyfile:xna4.pub $< -target:library -r:FNA.dll -r:FNA.NetStub.dll -out:$@
	sn -R $@ ../mono.snk

all: $(TARGETS)

FNA.dll: ../bin/Release/FNA.dll
	cp ../bin/Release/FNA.dll .

FNA.NetStub.dll: ../../FNA.NetStub/bin/Strongname/FNA.NetStub.dll
	cp ../../FNA.NetStub/bin/Strongname/FNA.NetStub.dll .

clean:
	rm -f $(TARGETS) FNA.dll FNA.NetStub.dll
