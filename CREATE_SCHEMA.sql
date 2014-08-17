CREATE TABLE pluviometer(
    created DATETIME,     -- date and time when transmission from pluviometer occured
    amount  DECIMAL(10,2) -- amount of rain in mm transmitted from pluviometer
);

CREATE TABLE anemometer(
    created DATETIME,     -- date and time when transmission from anemometer occured
    temperature  DECIMAL(4,2) -- temperature in Celsius degrees transmitted from pluviometer
);
