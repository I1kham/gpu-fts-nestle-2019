#ifndef _EVADTSParser_h_
#define _EVADTSParser_h_
#include "../rheaCommonLib/rhea.h"
#include "../rheaCommonLib/rheaFastArray.h"

/*********************************************************
 * EVADTSParser
 */
class EVADTSParser
{
private:
	typedef struct sTempStr128 { char s[128]; } TempStr128;

public:
	static const u8 NUM_LISTE_PREZZI = 4;
	static const u8 NUM_PAYMENT_DEVICE = 4;

	enum ePaymentDevice
	{
		ePaymentDevice_cash = 0,           //equivale a CA in evadts
		ePaymentDevice_cashless1 = 1,      //equivale a DA in evadts
		ePaymentDevice_cashless2 = 2,      //equivale a DB in evadts
		ePaymentDevice_token = 3,          //equivale a TA in evadts
		ePaymentDevice_unknown = 99
	};

	class ContatoreValNumValNum
	{
	public:
								ContatoreValNumValNum()										{ reset(); }
		void					reset()														{ val_tot = num_tot = val_par = num_par = 0; }
		void					valorizzaFromString_ValNumValNum(const TempStr128 *s)
		{
			val_tot = rhea::string::convert::toU32(s[0].s);
			num_tot = rhea::string::convert::toU32(s[1].s);
			val_par = rhea::string::convert::toU32(s[2].s);
			num_par = rhea::string::convert::toU32(s[3].s);
		}
		void					valorizzaFromString_NumValNumVal(const TempStr128 *s)
		{
			num_tot = rhea::string::convert::toU32(s[0].s);
			val_tot = rhea::string::convert::toU32(s[1].s);
			num_par = rhea::string::convert::toU32(s[2].s);
			val_par = rhea::string::convert::toU32(s[3].s);
		}

		ContatoreValNumValNum&	
operator += (const ContatoreValNumValNum &b)
		{
			num_tot += b.num_tot;
			num_par += b.num_par;
			val_par += b.val_par;
			val_tot += b.val_tot;
			return *this;
		}

	public:
		u32 val_tot;
		u32 num_tot;
		u32 val_par;
		u32 num_par;
	};

	/* MatriceContatori
	 *
	 * Sulle colonne ci sono i 4 possibili metodi di pagamento (CA DA DB TA), sulle righe ci sono le 4 possibili liste prezzo
	 *
	 *                | CA  |   DA  |   DB  |   TA  |
	 * ------------------------------------------------------
	 *                |     |       |       |       |       |
	 * ListaPrezzi 1  |     |       |       |       |  Tot1 |
	 *                |     |       |       |       |       |
	 * ------------------------------------------------------
	 *                |     |       |       |       |       |
	 * ListaPrezzi 2  |     |       |       |       |  Tot2 |
	 *                |     |       |       |       |       |
	 * ------------------------------------------------------
	 *                |     |       |       |       |       |
	 * ListaPrezzi 3  |     |       |       |       |  Tot3 |
	 *                |     |       |       |       |       |
	 * ------------------------------------------------------
	 *                |     |       |       |       |       |
	 * ListaPrezzi 4  |     |       |       |       | Tot4  |
	 *                |     |       |       |       |       |
	 * ------------------------------------------------------
	 *                |     |       |       |       |       |
	 *                |  A  |   B   |   C   |   D   |  ZZZ  |
	 *                |     |       |       |       |       |
	 * ------------------------------------------------------
	 *
	 * Ogni elemento della matrice è un ContatoreValNumValNum.
	 *  Tot1 = totale per la lista prezzi 1 (vedi fn totaliPerListaPrezzo())
	 *  Tot2 = totale per la lista prezzi 2 (vedi fn totaliPerListaPrezzo())
	 *  Tot3 = totale per la lista prezzi 3 (vedi fn totaliPerListaPrezzo())
	 *  Tot4 = totale per la lista prezzi 4 (vedi fn totaliPerListaPrezzo())
	 *
	 *  A = totale per metodo di pagamento CA (vedi fn totaliPerPaymentDevice())
	 *  B = totale per metodo di pagamento DA (vedi fn totaliPerPaymentDevice())
	 *  C = totale per metodo di pagamento DB (vedi fn totaliPerPaymentDevice())
	 *  D = totale per metodo di pagamento TA (vedi fn totaliPerPaymentDevice())
	 *
	 *  ZZZ = totale di tutti i metodi di pagamento e tutte le liste prezzo (vedi fn totaleDeiTotali())
	 */
	class MatriceContatori
	{
	public:
										MatriceContatori()
										{
											allocator = rhea::memory_getDefaultAllocator();
											m = (ContatoreValNumValNum**)RHEAALLOC(allocator, NUM_LISTE_PREZZI * NUM_PAYMENT_DEVICE * sizeof(ContatoreValNumValNum*));
											for (u16 i = 0; i < NUM_LISTE_PREZZI * NUM_PAYMENT_DEVICE; i++)
												m[i] = RHEANEW(allocator, ContatoreValNumValNum)();
											reset();
										}
										~MatriceContatori()
										{
											for (u16 i = 0; i < NUM_LISTE_PREZZI * NUM_PAYMENT_DEVICE; i++)
												RHEADELETE(allocator, m[i]);
											RHEAFREE(allocator, m);
										}

		void							reset()
		{
			for (u16 i = 0; i < NUM_LISTE_PREZZI * NUM_PAYMENT_DEVICE; i++)
				m[i]->reset();
		}

										//[iListaPrezzo] va 1 a NUM_LISTE_PREZZI
		void							set(int iListaPrezzo, ePaymentDevice paymentDevice, int num_tot, int val_tot, int num_par, int val_par)
		{
			if (iListaPrezzo < 1)
				return;
			if (iListaPrezzo > NUM_LISTE_PREZZI)
				return;
			iListaPrezzo--;

			int i2 = (int)paymentDevice;
			if (i2 >= NUM_PAYMENT_DEVICE)
				return;

			m[iListaPrezzo * NUM_PAYMENT_DEVICE + i2]->num_tot = num_tot;
			m[iListaPrezzo * NUM_PAYMENT_DEVICE + i2]->val_tot = val_tot;
			m[iListaPrezzo * NUM_PAYMENT_DEVICE + i2]->num_par = num_par;
			m[iListaPrezzo * NUM_PAYMENT_DEVICE + i2]->val_par = val_par;
		}

										//[iListaPrezzo] va 1 a NUM_LISTE_PREZZI
		void							add(int iListaPrezzo, ePaymentDevice paymentDevice, const ContatoreValNumValNum *c)
		{
			if (iListaPrezzo < 1)
				return;
			if (iListaPrezzo > NUM_LISTE_PREZZI)
				return;
			iListaPrezzo--;

			int i2 = (int)paymentDevice;
			if (i2 >= NUM_PAYMENT_DEVICE)
				return;

			*m[iListaPrezzo * NUM_PAYMENT_DEVICE + i2] += *c;
		}

										//[iListaPrezzo] va 1 a NUM_LISTE_PREZZI
		const ContatoreValNumValNum*	get(int iListaPrezzo, ePaymentDevice paymentDevice) const 
		{
			if (iListaPrezzo < 1)
				return &contatoreAZero;

			if (iListaPrezzo > NUM_LISTE_PREZZI)
				return &contatoreAZero;

			u8 i2 = (u8)paymentDevice;
			if (i2 >= NUM_PAYMENT_DEVICE)
				return &contatoreAZero;

			iListaPrezzo--;
			return m[iListaPrezzo * NUM_PAYMENT_DEVICE + i2];
		}

										//[iListaPrezzo] va 1 a NUM_LISTE_PREZZI
		void							getTotaliPerListaPrezzo_Tot_1_4 (int iListaPrezzo, ContatoreValNumValNum &out) const
		{
			out.reset();
			if (iListaPrezzo > 0 && iListaPrezzo <= NUM_LISTE_PREZZI)
			{
				iListaPrezzo--;
				for (u8 i = 0; i < NUM_PAYMENT_DEVICE; i++)
					out += *m[iListaPrezzo * NUM_PAYMENT_DEVICE + i];
			}
		}

										//[iListaPrezzo] va 1 a NUM_LISTE_PREZZI
		void							getTotaliPerPaymentDevice_ABCD(ePaymentDevice paymentDevice, ContatoreValNumValNum &out) const 
		{
			out.reset();
			u8 i2 = (u8)paymentDevice;
			if (i2 < NUM_PAYMENT_DEVICE)
			{
				for (u8 i = 0; i < NUM_LISTE_PREZZI; i++)
					out += *m[i * NUM_PAYMENT_DEVICE + i2];
			}
		}

										//[iListaPrezzo] va 1 a NUM_LISTE_PREZZI
		void							getTotaleDeiTotali_ZZZZ(ContatoreValNumValNum &out) const
		{
			out.reset();
			for (u8 i = 0; i < NUM_LISTE_PREZZI; i++)
			{
				for (u8 t = 0; t < NUM_PAYMENT_DEVICE; t++)
					out += *m[i *NUM_PAYMENT_DEVICE + t];
			}
		}

	private:
		static ContatoreValNumValNum	contatoreAZero;

	private:
		rhea::Allocator					*allocator;
		ContatoreValNumValNum			**m;
	};


	/* Per ogni selezioni, contiene tutte le informazioni raccolte analizzando il file evadts
	 *
	 * Viene popolata anazlizzando i tag PA1..PA7
	 */
	class InfoSelezione
	{
	public:
						InfoSelezione()								{ reset(); }
		void			reset()
						{
							id = price = 0;
							memset(name.s, 0, sizeof(name.s));
							aPagamento.reset();
							testvend.reset();
							freevend.reset();
							matriceContatori.reset();
						}

	public:
		int id;
		int price;
		TempStr128 name;
		ContatoreValNumValNum aPagamento;		//contatore num/val delle sel di questo tipo che sono state pagate, indipendentemente dal metodo di pagamento
		ContatoreValNumValNum testvend;			//come sopra ma per il "test vend"
		ContatoreValNumValNum freevend;			//come sopra ma per il "free vend"
		MatriceContatori matriceContatori;      //relativa alla singola selezione
	};


public:
						EVADTSParser();
						~EVADTSParser();

	bool				loadAndParse (const char *fullFilePathAndName);
	bool				parseFromMemory (const u8 *buffer, u32 firstByte, u32 nBytesToCheck);

	u8*					createBufferWithFormattedData(rhea::Allocator *allocator, u32 *out_bufferLen) const;


					/*	QUERY
					 * le seguenti variabili vengono valorizzate a seguito di un load() o parse()
					 * Sono esposte e sono pubbliche ma l'intento è che siano di sola lettura quindi l'utilizzatore di questa classe dovrebbe semplicemente
					 * leggere ed evitare di scriverle
					 */
	ContatoreValNumValNum VA1;			//relativo alle selezioni che sono state pagate, indipendentemente dal metodo di pagamento
	ContatoreValNumValNum VA2;			//come sopra ma per il "test vend"
	ContatoreValNumValNum VA3;			//come sopra ma per il "free vend"
	ContatoreValNumValNum CA2;			//relativo alle vendite pagate con CASH
	MatriceContatori matriceContatori;  //relativa a tutte le selezioni sommate
	rhea::FastArray<InfoSelezione*> selezioni;	//dettaglio di ogni singola selezione

private:
	void				priv_reset();
	bool				priv_checkTag(const char *s, const char *tagToFindAtStartOfTheLine, u16 numOfParamsToRead, TempStr128 *out) const;
	int					priv_toInt(const char *s) const;
	ePaymentDevice		priv_fromStringToPaymentDevice(const char *s) const;

	private:
		rhea::Allocator	*allocator;
};

#endif // _EVADTSParser_h_
