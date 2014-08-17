#ifndef AURIOL_DB_H_
#define AURIOL_DB_H_

void initializeDatabase();
void savePluviometer(float amountInMilimeters);
void saveAnemometerTemperatureAndHumidity(float temperatureInC);

#endif
