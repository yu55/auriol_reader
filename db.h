#ifndef AURIOL_DB_H_
#define AURIOL_DB_H_

void initializeDatabase();
void savePluviometer(float amount);
void saveTemperature(float temperature);
void saveHumidity(unsigned int humidity);
void saveWind(float speed, float gust, unsigned int direction);

#endif
