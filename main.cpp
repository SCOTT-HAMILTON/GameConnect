#include <iostream>
#include <time.h>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace sf;

#define WIDTH 300
#define HEIGTH 300
#define SIZE_PERSO 20


const int PING = 217859;
int FIRST;
int SECOND;
const int REPONSE = 6574;

int pointPerso = 0, pointPersoPre = 0;

int pointPerso2 = 0;

int SpeedPerso = 5;
int nbticksAct = 0;
int coeff = 0;

std::string signPosS("MY POS"), signPosR, signSpriteS("SPRITE POS"), signSpriteR;
std::string stringPtPerso("joueur 1 : "), stringPtPerso2("joueur 2 : ");
IpAddress addressS;
UdpSocket socket, socketSprite;
IpAddress addressR;
IpAddress addressL = IpAddress::getPublicAddress();
Packet packetS, packetR, packetSpriteS, packetSpriteR;
unsigned short portL = 2000, portSprite = 2500;
Vector2f posPerso, posPerso2R, posPerso2(-1, -1), posSprite;
Time ticksActu;
Clock c;
bool spriteExist = false;

Vector2f posSpriteR, posSpriteS;
FILE *fichier = fopen("error.txt", "w");

bool touch(Vector2f pos1, Vector2f pos2);
void connect();
void receive(int *data);
void receivePos();
void receiveSprite();

int main()
{
    connect();

    RenderWindow fenetre(VideoMode(WIDTH, HEIGTH), "GameConnect");
    Event event;
    Color fenetreColor(36, 200, 89), persoColor(31, 116, 167), persoRColor(255, 10, 15), spriteColor(223, 235, 73);
    RectangleShape perso(Vector2f(SIZE_PERSO, SIZE_PERSO));
    RectangleShape persoR(Vector2f(SIZE_PERSO, SIZE_PERSO));
    Sprite sprite;
    Image imgSprite;
    imgSprite.create(SIZE_PERSO, SIZE_PERSO);
    imgSprite.loadFromFile("sprite.bmp");
    imgSprite.createMaskFromColor(Color(255, 255, 255));
    Texture textSprite;
    textSprite.create(SIZE_PERSO, SIZE_PERSO);
    textSprite.update(imgSprite);
    sprite.setTexture(textSprite);
    perso.setFillColor(persoColor);
    persoR.setFillColor(persoRColor);

    Font fontText;
    if (!fontText.loadFromFile("arial.ttf")) std::cout << "textError !" << std::endl;

    Text textPtperso, textPtperso2;
    textPtperso.setFont(fontText);
    textPtperso2.setFont(fontText);
    char tmpPoint[3];
    char tmpPoint2[3];
    sprintf(tmpPoint2, "%d", pointPerso2);
    sprintf(tmpPoint, "%d", pointPerso);
    stringPtPerso.append(tmpPoint);
    stringPtPerso2.append(tmpPoint2);
    textPtperso.setString(stringPtPerso);
    textPtperso2.setString(stringPtPerso2);
    textPtperso2.setCharacterSize(10);
    textPtperso.setCharacterSize(10);
    textPtperso2.setColor(Color(0, 0, 0));
    textPtperso.setColor(Color(0, 0, 0));
    textPtperso2.setPosition(Vector2f(WIDTH - textPtperso2.getLocalBounds().width - 10, 10));
    textPtperso.setPosition(Vector2f(10, 10));

    srand(time(NULL));
    time_t tmpAct, tmpPrec;
    Vector2f posPersoTmp;
    posPerso.x = rand()%((WIDTH-SIZE_PERSO)-0)+0;
    posPerso.y = rand()%((HEIGTH-SIZE_PERSO)-0)+0;
    int i = 0;

    packetS << posPerso.x << posPerso.y;
    std::cout << "posPersoSend : " << posPerso.x << "|" << posPerso.y << std::endl;
    socket.send(packetS, addressR, portL);
    unsigned short portR;
    socket.setBlocking(false);
    tmpPrec = time(NULL);
    tmpAct = tmpPrec;
    while ((posPerso2.x == -1 || posPerso2.y == -1)&& tmpAct-tmpPrec < 10){
        tmpAct = time(NULL);
        socket.receive(packetR, addressR, portR);
        packetR >> posPerso2.x >> posPerso2.y;
    }
    while (touch(posPerso, posPerso2)){
        posPerso.x = rand()%((WIDTH-SIZE_PERSO)-0)+0;
        posPerso.y = rand()%((HEIGTH-SIZE_PERSO)-0)+0;
        packetS << signPosS << posPerso.x << posPerso.y;
        socket.send(packetS, addressR, portL);
        while ((posPerso2.x == -1 || posPerso2.y == -1)&& tmpAct-tmpPrec < 10){
        tmpAct = time(NULL);
        socket.receive(packetR, addressR, portR);
        packetR >> signPosR >> posPerso2.x >> posPerso2.y;
        }
    }

     std::cout << "posPersoR : " << posPerso2.x << ", "<<posPerso2.y << std::endl;
    if (posPerso2.x == -1){
        std::cout << "pos receive error!" << std::endl;
        getchar();
        return EXIT_FAILURE;
    }
    socket.send(packetS, addressR, portL);
    socket.setBlocking(true);

    posPersoTmp = posPerso;
    perso.setPosition(posPerso);
    persoR.setPosition(posPerso2);
    Thread threadR(&receivePos);
    threadR.launch();

    Thread threadSprite(&receiveSprite);
    threadSprite.launch();

    while (fenetre.isOpen()){
        posPersoTmp = posPerso;
        ticksActu = c.getElapsedTime();
        if (ticksActu.asMilliseconds()%15 == 0){
            nbticksAct++;
            if (FIRST && !spriteExist){
                fprintf (fichier, "\t\t\t\t FIRST! \n");
                srand(time(NULL) * (nbticksAct+2));
                posSprite.x = rand()%((WIDTH-SIZE_PERSO)-0)+0;
                posSprite.y = rand()%((HEIGTH-SIZE_PERSO)-0)+0;
                i = 1;
                while (touch(posPerso, posSprite) || touch(posPerso2, posSprite)){
                    srand(time(NULL) * (nbticksAct+2)* i);
                    posSprite.x = rand()%((WIDTH-SIZE_PERSO)-0)+0;
                    posSprite.y = rand()%((HEIGTH-SIZE_PERSO)-0)+0;
                    i+=2;
                }
                packetSpriteS.clear();
                posSpriteS.x = posSprite.x;
                posSpriteS.y = posSprite.y;
                packetSpriteS << signSpriteS << posSpriteS.x << posSpriteS.y;
                socketSprite.send(packetSpriteS, addressS, portSprite);
                spriteExist = true;

            }
            while (fenetre.pollEvent(event)){
                if (event.type == Event::Closed)fenetre.close();

                if (Keyboard::isKeyPressed(Keyboard::Up)){
                    std::cout << "Up!" << std::endl;
                    posPersoTmp.y -= SpeedPerso;
                }

                if (Keyboard::isKeyPressed(Keyboard::Down)){
                    std::cout << "Down!" << std::endl;
                    posPersoTmp.y += SpeedPerso;
                }

                if (Keyboard::isKeyPressed(Keyboard::Right)){
                    std::cout << "Right!" << std::endl;
                    posPersoTmp.x += SpeedPerso;
                }

                if (Keyboard::isKeyPressed(Keyboard::Left)){
                    std::cout << "Left!" << std::endl;
                    posPersoTmp.x -= SpeedPerso;
                }
            }
            if ((posPersoTmp.x < 0 || posPersoTmp.x+SIZE_PERSO > WIDTH)){
                std::cout << "x!" << std::endl;
            }else if((posPersoTmp.y < 0 || posPersoTmp.y+SIZE_PERSO > HEIGTH))std::cout << "y!" << std::endl;
            else if (touch(posPersoTmp, posPerso2)) printf("touche\n");
            else{
                posPerso = posPersoTmp;
            }

            if (touch(posPerso, posSprite)){
                if (!FIRST && pointPersoPre == pointPerso){
                    spriteExist = false;
                    pointPerso++;
                }else if (FIRST){
                    spriteExist = false;
                    pointPerso++;
                }

            }

            if (touch(posPerso2, posSprite)){
                spriteExist = false;
                pointPerso2++;
            }

            packetS.clear();
            packetS << signPosS << posPerso.x << posPerso.y;
            std::cout << "posPersoSend : " << posPerso.x << "|" << posPerso.y << std::endl;
            socket.send(packetS, addressS, portL);
            perso.setPosition(posPerso);
            stringPtPerso = "joueur 1 : ";
            stringPtPerso2 = "joueur 2 : ";
            sprintf(tmpPoint2, "%d", pointPerso2);
            sprintf(tmpPoint, "%d", pointPerso);
            stringPtPerso.append(tmpPoint);
            stringPtPerso2.append(tmpPoint2);
            textPtperso.setString(stringPtPerso);
            textPtperso2.setString(stringPtPerso2);
        }
        persoR.setPosition(posPerso2);
        sprite.setPosition(posSprite);

        fenetre.draw(sprite);
        fenetre.draw(persoR);
        fenetre.draw(perso);
        fenetre.draw(textPtperso2);
        fenetre.draw(textPtperso);
        fenetre.display();
        fenetre.clear(fenetreColor);
        ticksActu = c.getElapsedTime();
    }
    return 0;
}

void receive(int *data){
    std::size_t sizeR;
    unsigned short portR;
    if (socket.receive(data, sizeof(data), sizeR, addressR, portR) == Socket::Error){
        std::cout << "data receive error!" << std::endl;
        getchar();
    }
}

void receivePos(){
    unsigned short portR;
    socket.setBlocking(true);
    while (1){
        socket.receive(packetR, addressR, portR);
        packetR >> signPosR>>posPerso2R.x >> posPerso2R.y;
        if (signPosR == signPosS){
            posPerso2.x = posPerso2R.x;
            posPerso2.y = posPerso2R.y;
        }
    }
}

void connect(){
    std::cout << IpAddress::getPublicAddress() << std::endl;
    if (addressL == IpAddress("5.51.51.245")){
        addressS = ("90.87.221.204");
    }else{
        addressS = ("5.51.51.245");
    }

    const int PING = 12363654;
    const int REPONSE = 675465748;

    int dataR = 0;

    socket.bind(portL);
    socketSprite.bind(portSprite);
    Thread thread(&receive, &dataR);
    thread.launch();
    socket.send(&PING, sizeof(&PING), addressS, portL);
    std::cout << "en attente..." << std::endl;
    while (dataR != PING){
        if (dataR == REPONSE){
            FIRST = 1;
            SECOND = 0;
            break;
        }
    }if (!FIRST)SECOND = 1;
    socket.send(&REPONSE, sizeof(&REPONSE), addressS, portL);
    std::cout << "vous etes connecte" << std::endl;
 }

 bool touch(Vector2f pos1, Vector2f pos2){
    if ((pos1.x+SIZE_PERSO) >= pos2.x && (pos1.x+SIZE_PERSO) <= (pos2.x+SIZE_PERSO)){
        if ((pos1.y+SIZE_PERSO)>= pos2.y && (pos1.y+SIZE_PERSO) <= (pos2.y+SIZE_PERSO)){
            return true;
        }
        if (pos1.y >= pos2.y && pos1.y <= (pos2.y+SIZE_PERSO)){
            return true;
        }
    }

    if (pos1.x >= pos2.x && pos1.x <= (pos2.x+SIZE_PERSO)){
        if ((pos1.y+SIZE_PERSO)>= pos2.y && (pos1.y+SIZE_PERSO) <= (pos2.y+SIZE_PERSO)){
            return true;
        }
        if (pos1.y >= pos2.y && pos1.y <= (pos2.y+SIZE_PERSO)){
            return true;
        }
    }
    return false;
 }

 void receiveSprite(){
     unsigned short portR;
    socketSprite.setBlocking(true);
    while (true){

        packetSpriteR.clear();
        socketSprite.receive(packetSpriteR, addressR, portR);
        packetSpriteR >> signSpriteR >> posSpriteR.x >> posSpriteR.y;
        if (signSpriteR == signSpriteS){
            posSprite.x = posSpriteR.x;
            posSprite.y = posSpriteR.y;
        }
        pointPersoPre = pointPerso;
    }
 }

