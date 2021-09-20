#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QVariant>
#include <QByteArray>

class QWidget;

#define defWebServerEndpoint "http://localhost:8080"

#define defWebAppEndpoint "https://prod-10.westus.logic.azure.com:443/workflows/3372030c305c406d8e53f39b3c5f33d3/triggers/manual/paths/invoke?api-version=2016-06-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=6B5BtuMb_WelO608JH5K0r4K9cxPGZCeUveEqLL3rdk"

#define defWebAppPublicKey ("MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAyp5GQFMeLTsNTGf6E1TPTfDXiJ/Pod/xqPaDVY3IV+CAPGxfVIgP/uBZm+ORgUe/FZ+9YykHYXYTatUeH2DSSCQ+XFKfVaEhVqOYEejxeHq505X0/Q6F1icL\r\nN28+VFWTuOTBhFCZIFVR7kjzpbgwoiFtx0Aa7fS/K6Sir0YCB56zTcynctuMYD4/welH/b0abFtP+TVQn7+x\r\nWVjP6MMXru/Wiog+sXdqk4n0BdLrRFpbdhpbrZcdUeVTlws2/RKqAQDJ5ws48WHgZJtGG/Ka7/IvrBOEx\r\nKo/ykJ+afKaBEhuvHsfQVTb1+TncYZanoC2EkH0Lh0idEyyP+F3/4hG6nWq2LF6/8L723oTNv4GvdB5dGt3\r\n5+5dhkH26Aplf/PSxFrEvbDYFsPFyANnepQRplSbNayHPEIcpFexDKjDCKnSMeu5yS/pXICPXOjhL/Vy6Mysi\r\n5arEhHA8NzD1mU78R8uOXAFypX2T6+02+NIHtDmGzM3amwLILMqm8TaW91CKFHeSd+JCCCJsvJLXrV\r\nmxt0DkBgiobAcCHyvfZ8tbPOtMdu2L8yscyAXJnQZXFw+BDSbM7Qppq0A8R7xO+3eOvG+3svwhHhEg/jY\r\niKOzm5q/LCzJ5zXQnvZrOmeaUUl2VxXW7GX6TqFZZdd9tWgmFil3bvALGxdf/LhBgZ5S1qUECAwEAAQ==")

void changeProperty(QWidget * obj, const char *property, QVariant value);

#endif // UTILS_H
