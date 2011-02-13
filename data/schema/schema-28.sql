UPDATE %allsongstables SET length=length*1e9, beginning=beginning*1e9;

UPDATE schema_version SET version=28;
