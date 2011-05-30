-(void)mediaKeyTap:(SPMediaKeyTap*)keyTap receivedMediaKeyEvent:(NSEvent*)event;
{
	assert([event type] == NSSystemDefined && [event subtype] == SPSystemDefinedEventMediaKeys);

	int keyCode = (([event data1] & 0xFFFF0000) >> 16);
	int keyFlags = ([event data1] & 0x0000FFFF);
	int keyState = (((keyFlags & 0xFF00) >> 8)) == 0xA;
	int keyRepeat = (keyFlags & 0x1);
		
	if (keyState == 1 && windowController != NULL) {
		
		
		switch (keyCode) {
				
			case NX_KEYTYPE_PLAY:
...				return;
				
			case NX_KEYTYPE_FAST:
...				return;
				
			case NX_KEYTYPE_REWIND:
...				return;
		}
	}
}
