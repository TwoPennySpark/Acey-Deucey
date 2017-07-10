#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>

#define DECK_NUM 51
#define TIMES_TO_SHUFFLE_ONE_DECK 5
#define SPADE   "\xE2\x99\xA0"
#define CLUB    "\xE2\x99\xA3"
#define HEART   "\xE2\x99\xA5"
#define DIAMOND "\xE2\x99\xA6"

typedef struct player_s
{
	char name[100];
	float bankroll;
	struct player_s* listp;
}player;

typedef struct card_s
{
	char* suit;
	int face;
	/*union Type
	{
		int face_num;
		char face_symb;
	}type;*/
	struct card_s* listp;
}card;

struct card_s* deck[DECK_NUM];

struct player_s* first_head;
struct player_s* player_after_player_to_del;

struct player_s* addPlayer(struct player_s*, char*);
struct player_s* deletePlayer(struct player_s*);
struct card_s* addCard(struct card_s*, char, char*);
void deckCreate(struct card_s*);
void showCardsInfo(struct card_s*);
void showPlayersInfo(struct player_s*);
void shuffle(void);
void game(struct player_s*, int);
void drawCard(int, char*);

int main(void)
{
	int players_num = 0, i = 0, fd = 0;
	struct player_s* head;
	struct card_s* first;
	char newname[100];

	srand(time(NULL));
	head = NULL;
	first = NULL;

	//if ((fd = open("game", O_CREAT | O_TRUNC | O_WRONLY, 0744)) < 0)
		//printf("[-]Can't open file\n");	

	printf("Enter the number of players total:\n");
	scanf("%d", &players_num);

	for (i = 0; i < players_num; i++)
	{  
		printf("Enter your name:\n");
		scanf("%s", newname);
		//printf("And your bankroll is:\n");
		//scanf("%d", &newbankroll);

		/*write(fd, newname, strlen(newname));
		sprintf(tmp, " \t%d$", newbankroll);
		write(fd, tmp, strlen(tmp));
		write(fd, "\n", 1);*/
		head = addPlayer(head, newname);		
		if (i == 0)
			first_head = head;
	}
	first_head->listp = head;

	printf("Current players information:\n");
	showPlayersInfo(head);

	deckCreate(first);
	showCardsInfo(first);
	shuffle();
	game(head, players_num);

	//close(fd);
	free(head);
	return 0;
}

void game(struct player_s* head, int players_num)
{
	int min = 0, max = 0;
	float bet = 0, pot = players_num * 5;
	int card_num = 0;

	while (players_num != 1 && pot > 0)
	{
		if (card_num >= DECK_NUM - 1)
		{
			printf("This deck of cards is over, new shuffle:\n");
			shuffle();
			card_num = 0;
		}
		if (deck[card_num]->face > deck[card_num + 1]->face)
			{max = deck[card_num]->face; min = deck[card_num + 1]->face;}
		else
			{min = deck[card_num]->face; max = deck[card_num + 1]->face;}
		printf("\n-------------------------------------------------------------------------------------------\n");
		printf("||Next two cards is for the player: %s\n||Player's bankroll is: %.2f$\n||Pot is: %.2f$\n", head->name, head->bankroll, pot);
		printf("-------------------------------------------------------------------------------------------\n");
		printf("The cards are:\n");
		printf("===========================================================================================\n");
		drawCard(deck[card_num]->face, deck[card_num]->suit);
		drawCard(deck[card_num + 1]->face, deck[card_num + 1]->suit);
		printf("===========================================================================================\n");
		printf("Player's bet is:\n");
		scanf("%f", &bet);	

		while(bet < 0 || bet > pot || bet > head->bankroll)
		{ 
			printf("[-]Remember that bet can't be negative number and exceed the amount of pot or player's bankroll!\n"); 
			printf("Player's bets:\n");
			scanf("%f", &bet);
		}
		card_num += 2;

		if (bet == 0.0)
		{
			printf("Player decides to pass this turn\n");
			card_num+=1;
			head = head->listp;
			continue;
		}

		printf("Pulling another card from the deck and it is...\n");
		sleep(1);
		drawCard(deck[card_num]->face, deck[card_num]->suit);

		if ((deck[card_num]->face < max) && (deck[card_num]->face > min))
		{		
			printf("[+]Player %s wins %.2f$!\n", head->name, bet);
		 	pot -= bet;
			head->bankroll += bet;
		}
		else 
		{		
			if ((deck[card_num]->face > max) || (deck[card_num]->face < min))
			{
				printf("[-]Bad luck, player %s loses his bet: -%.2f$\n", head->name, bet);
				pot += bet;
				head->bankroll -= bet;
			}
			if ((deck[card_num]->face == min) || (deck[card_num]->face == max))
			{
				if (head->bankroll >= bet*2)
				{					
					pot += bet*2;
					head->bankroll -= bet*2;
					printf("[-]Bad luck, player %s loses double bet: -%.2f$\n", head->name, bet*2);
				}
				else
				{
					printf("[-]Bad luck, player %s loses the rest of his money: -%.2f$\n", head->name, head->bankroll);
					pot += head->bankroll;
					head->bankroll = 0;
				}
			}
		}
		card_num += 1;
		if (head->bankroll <= 0)
		{
			printf("||Player %s is out of founds\n", head->name);
			head = deletePlayer(head);
			players_num--;
			continue;
		}
		head = head->listp;
	}	
	if (pot <= 0)
	{
		printf("Pot is empty! Game ended with the following results:\n");
		showPlayersInfo(head);
	}
	else
		printf("\n[!]And the winner is: %s with a bankroll of %.2f$!\n", head->name, head->bankroll + pot);
}

struct player_s* deletePlayer(struct player_s* head)
{
	struct player_s* playerIterator = first_head->listp;
	if (head == first_head)
	{
		player_after_player_to_del = playerIterator;
		while(playerIterator->listp != first_head)
		{ 
			player_after_player_to_del = playerIterator;
			playerIterator = playerIterator->listp;
		}
		playerIterator->listp = first_head->listp;
		return first_head->listp;
	}
	else
	{
		player_after_player_to_del = first_head;
		while(playerIterator != head)
		{
			player_after_player_to_del = playerIterator;
			playerIterator = playerIterator->listp;
		}
		if (player_after_player_to_del->listp != player_after_player_to_del)
			player_after_player_to_del->listp = playerIterator->listp;
		else
			return NULL;
		return playerIterator->listp;
	}
}

struct player_s* addPlayer(struct player_s* head, char * newname)
{
	int len = strlen(newname);
	struct player_s* newplayer;
	newplayer = (struct player_s*)malloc(sizeof(struct player_s));
	strncpy(newplayer->name, newname, len);
	newplayer->bankroll = 100.0;
	newplayer->listp = head;
	return newplayer; 
}

struct card_s* addCard(struct card_s* head, char cardFace, char* cardSuit)
{
	struct card_s* newCard; 
	newCard = (struct card_s*)malloc(sizeof(struct card_s));
	newCard->suit = cardSuit;
	newCard->face = cardFace;
	newCard->listp = head;
	return newCard;
}
	
void deckCreate(struct card_s* head)
{
	int i = 0, j = 0, k = 0;
	char *suits[] = {CLUB, SPADE, DIAMOND, HEART}; 
	//head = NULL;
	
	for (i = 0; i < 4; i++)
	{
		for (j = 2; j <= 14; j++)
		{
			//printf("[%d]:%d%s\n", k, j, suits[i]); 
			head = addCard(head, j, suits[i]);	
			deck[k] = head;
			k++;
		}
	}	
}

void showCardsInfo(struct card_s* head)
{
	while(head != NULL)
	{
		//printf("%s%d\n", head->suit, head->face);
		drawCard(head->face, head->suit);
		head = head->listp;
	}
}

void showPlayersInfo(struct player_s* head)
{
	int fd = 0;
	char tmp[64] = {0};
	struct player_s* headIterator = first_head->listp;

	if ((fd = open("player_info", O_CREAT | O_TRUNC | O_WRONLY, 0744)) < 0)
		printf("[-]Can't open the file\n");	

	printf("===========================================================================================\n");
	while(headIterator != first_head)
	{
		printf("Name\t:%s\nBankroll:%.2f$\n", headIterator->name, headIterator->bankroll);
		sprintf(tmp, "%s\t%.2f$\n",headIterator->name, headIterator->bankroll);
		write(fd, tmp, strlen(tmp));
		headIterator = headIterator->listp;
	}
	printf("Name\t:%s\nBankroll:%.2f$\n", first_head->name, first_head->bankroll);
	sprintf(tmp, "%s\t%.2f$\n",headIterator->name, headIterator->bankroll);
	write(fd, tmp, strlen(tmp));
	close(fd);
} 

void drawCard(int faceWeight, char* suit)
{
	char face;
	switch(faceWeight)
	{
		case 10:
		{face = 10;
		 printf("---------\n|%d%s\t|\n|\t|\n|\t|\n|    %d%s|\n---------\n", face, suit ,face, suit);
		 return;}
		case 11:
		{face = 'J'; break;}
		case 12:
		{face = 'Q'; break;}
		case 13:
		{face = 'K'; break;}
		case 14: 
		{face = 'A'; break;}
		default:
		{face = faceWeight + '0';break;}
	}
	printf("---------\n|%c%s\t|\n|\t|\n|\t|\n|     %c%s|\n---------\n", face, suit ,face, suit);
}

void shuffle(void)
{
	int i = 0, j = 0, random = 0;
	struct card_s* temperary_var; 
	for (j = 0; j < TIMES_TO_SHUFFLE_ONE_DECK; j++)
	{
		for (i = 0; i < DECK_NUM; i++)
		{
		    random = rand()%52;
			temperary_var = deck[i];
			deck[i] = deck[random];
			deck[random] = temperary_var;
		}	
	}
	for (i = 0;i <= DECK_NUM; i++)
		printf(" %s%d", deck[i]->suit, deck[i]->face);
}
