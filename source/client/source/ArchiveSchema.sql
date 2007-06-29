create table Settings ([Name] text, [intValue] int, [stringValue] text);
create table Screenshots ([id] INTEGER PRIMARY KEY AUTOINCREMENT, [bitmapData] blob, [thumbnailData] blob, [width] int, [height] int, [date] text);
create table Events ([id] INTEGER PRIMARY KEY AUTOINCREMENT, screenshotID int, icon int, eventType int, destinationName text, eventText text, url text, [date] text);
