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

FNA_DLL = WineMono.FNA.dll
FNA_NETSTUB_DLL = WineMono.FNA.NetStub.dll

%.dll: %.cs $(FNA_DLL) $(FNA_NETSTUB_DLL)
	mkdir -p bin
	mcs -delaysign+ -keyfile:xna4.pub $< -target:library -r:$(FNA_DLL) -r:$(FNA_NETSTUB_DLL) -out:$@
	sn -R $@ ../mono.snk

all: $(TARGETS)

$(FNA_DLL): ../bin/Release/$(FNA_DLL)
	cp ../bin/Release/$(FNA_DLL) .

$(FNA_NETSTUB_DLL): ../../FNA.NetStub/bin/Strongname/$(FNA_NETSTUB_DLL)
	cp ../../FNA.NetStub/bin/Strongname/$(FNA_NETSTUB_DLL) .

clean:
	rm -f $(TARGETS) $(FNA_DLL) $(FNA_NETSTUB_DLL)
