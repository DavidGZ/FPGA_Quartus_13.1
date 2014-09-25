/*
 * URB OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2001 David Brownell <dbrownell@users.sourceforge.net>
 *
 * usb-ohci.h
 */
 
#define USB_OHCI_MAX_ROOT_PORTS	4

static int cc_to_error[16] =
{

/* mapping of the OHCI CC status to error codes */
	/* No  Error  */	       0,
	/* CRC Error  */	       USB_ST_CRC_ERR,
	/* Bit Stuff  */	       USB_ST_BIT_ERR,
	/* Data Togg  */	       USB_ST_CRC_ERR,
	/* Stall      */	       USB_ST_STALLED,
	/* DevNotResp */	       -1,
	/* PIDCheck   */	       USB_ST_BIT_ERR,
	/* UnExpPID   */	       USB_ST_BIT_ERR,
	/* DataOver   */	       USB_ST_BUF_ERR,
	/* DataUnder  */	       USB_ST_BUF_ERR,
	/* reservd    */	       -1,
	/* reservd    */	       -1,
	/* BufferOver */	       USB_ST_BUF_ERR,
	/* BuffUnder  */	       USB_ST_BUF_ERR,
	/* Not Access */	       -1,
	/* Not Access */	       -1
};

#ifdef DEBUG_OHCI
static const char *cc_to_string[16] =
{
	"No Error",
	"CRC: Last data packet from endpoint contained a CRC error.",
	"BITSTUFFING:\r\nLast data packet from endpoint contained a bit stuffing violation",
	"DATATOGGLEMISMATCH:\r\n Last packet from endpoint had data toggle PID\r\n" \
		     "that did not match the expected value.",
	"STALL: TD was moved to the Done Queue because the endpoint returned a STALL PID",
	"DEVICENOTRESPONDING:\r\nDevice did not respond to token (IN) or did\r\n" \
		     "not provide a handshake (OUT)",
	"PIDCHECKFAILURE:\r\nCheck bits on PID from endpoint failed on data PID\r\n"\
		     "(IN) or handshake (OUT)",
	"UNEXPECTEDPID:\r\nReceive PID was not valid when encountered or PID\r\n" \
		     "value is not defined.",
	"DATAOVERRUN:\r\nThe amount of data returned by the endpoint exceeded\r\n" \
		     "either the size of the maximum data packet allowed\r\n" \
		     "from the endpoint (found in MaximumPacketSize field\r\n" \
		     "of ED) or the remaining buffer size.",
	"DATAUNDERRUN:\r\nThe endpoint returned less than MaximumPacketSize\r\n" \
		     "and that amount was not sufficient to fill the\r\n" \
		     "specified buffer",
	"reserved1",
	"reserved2",
	"BUFFEROVERRUN:\r\nDuring an IN, HC received data from endpoint faster\r\n" \
		     "than it could be written to system memory",
	"BUFFERUNDERRUN:\r\nDuring an OUT, HC could not retrieve data from\r\n" \
		     "system memory fast enough to keep up with data USB data rate.",
	"NOT ACCESSED:\r\nThis code is set by software before the TD is placed\r\n" \
		     "on a list to be processed by the HC.(1)",
	"NOT ACCESSED:\r\nThis code is set by software before the TD is placed\r\n" \
		     "on a list to be processed by the HC.(2)",
};
#endif /* DEBUG_OHCI */

/* ED States */

#define ED_NEW		0x00
#define ED_UNLINK	0x01
#define ED_OPER		0x02
#define ED_DEL		0x04
#define ED_URB_DEL	0x08

/* usb_ohci_ed */
struct ed
{
	uint32_t hwINFO;
	uint32_t hwTailP;
	uint32_t hwHeadP;
	uint32_t hwNextED;

	struct ed *ed_prev;
	uint8_t int_period;
	uint8_t int_branch;
	uint8_t int_load;
	uint8_t int_interval;
	uint8_t state;
	uint8_t type;
	uint16_t last_iso;
	struct ed *ed_rm_list;

	struct usb_device *usb_dev;
	void *purb;
	uint32_t unused[2];
} __attribute__((aligned(16)));
typedef struct ed ed_t;


/* TD info field */
#define TD_CC	    0xf0000000
#define TD_CC_GET(td_p) ((td_p >>28) & 0x0f)
#define TD_CC_SET(td_p, cc) (td_p) = ((td_p) & 0x0fffffff) | (((cc) & 0x0f) << 28)
#define TD_EC	    0x0C000000
#define TD_T	    0x03000000
#define TD_T_DATA0  0x02000000
#define TD_T_DATA1  0x03000000
#define TD_T_TOGGLE 0x00000000
#define TD_R	    0x00040000
#define TD_DI	    0x00E00000
#define TD_DI_SET(X) (((X) & 0x07)<< 21)
#define TD_DP	    0x00180000
#define TD_DP_SETUP 0x00000000
#define TD_DP_IN    0x00100000
#define TD_DP_OUT   0x00080000

#define TD_ISO	    0x00010000
#define TD_DEL	    0x00020000

/* CC Codes */
#define TD_CC_NOERROR	   0x00
#define TD_CC_CRC	   0x01
#define TD_CC_BITSTUFFING  0x02
#define TD_CC_DATATOGGLEM  0x03
#define TD_CC_STALL	   0x04
#define TD_DEVNOTRESP	   0x05
#define TD_PIDCHECKFAIL	   0x06
#define TD_UNEXPECTEDPID   0x07
#define TD_DATAOVERRUN	   0x08
#define TD_DATAUNDERRUN	   0x09
#define TD_BUFFEROVERRUN   0x0C
#define TD_BUFFERUNDERRUN  0x0D
#define TD_NOTACCESSED	   0x0F


#define MAXPSW 1

struct td
{
	uint32_t hwINFO;
	uint32_t hwCBP;		/* Current Buffer Pointer */
	uint32_t hwNextTD;		/* Next TD Pointer */
	uint32_t hwBE;		/* Memory Buffer End Pointer */

	uint16_t hwPSW[MAXPSW];
	uint8_t unused;
	uint8_t index;
	struct ed *ed;
	struct td *next_dl_td;
	struct usb_device *usb_dev;
	int transfer_len;
	uint32_t data;

	uint32_t unused2[2];
} __attribute__((aligned(32)));
typedef struct td td_t;

#define OHCI_ED_SKIP	  (1 << 14)

/*
 * The HCCA (Host Controller Communications Area) is a 256 byte
 * structure defined in the OHCI spec. that the host controller is
 * told the base address of.  It must be 256-byte aligned.
 */

#define NUM_INTS 32	/* part of the OHCI standard */
struct ohci_hcca
{
	uint32_t	int_table[NUM_INTS];	/* Interrupt ED table */
#if defined(CONFIG_MPC5200)
	uint16_t	pad1;			/* set to 0 on each frame_no change */
	uint16_t	frame_no;		/* current frame number */
#else
	uint16_t	frame_no;		/* current frame number */
	uint16_t	pad1;			/* set to 0 on each frame_no change */
#endif
	uint32_t	done_head;		/* info returned for an interrupt */
	uint8_t		reserved_for_hc[116];
} __attribute__((aligned(256)));

/*
 * This is the structure of the OHCI controller's memory mapped I/O
 * region.  This is Memory Mapped I/O.	You must use the readl() and
 * writel() macros defined in asm/io.h to access these!!
 */
struct ohci_regs
{
	/* control and status registers */
	uint32_t	revision;
	uint32_t	control;
	uint32_t	cmdstatus;
	uint32_t	intrstatus;
	uint32_t	intrenable;
	uint32_t	intrdisable;
	/* memory pointers */
	uint32_t	hcca;
	uint32_t	ed_periodcurrent;
	uint32_t	ed_controlhead;
	uint32_t	ed_controlcurrent;
	uint32_t	ed_bulkhead;
	uint32_t	ed_bulkcurrent;
	uint32_t	donehead;
	/* frame counters */
	uint32_t	fminterval;
	uint32_t	fmremaining;
	uint32_t	fmnumber;
	uint32_t	periodicstart;
	uint32_t	lsthresh;
	/* Root hub ports */
    struct	ohci_roothub_regs
    {
		uint32_t	a;
		uint32_t	b;
		uint32_t	status;
		uint32_t	portstatus[USB_OHCI_MAX_ROOT_PORTS];
	} roothub;
} __attribute__((aligned(32)));

/* Some EHCI controls */
#define EHCI_USBCMD_OFF		0x20
#define EHCI_USBCMD_HCRESET	(1 << 1)

/* OHCI CONTROL AND STATUS REGISTER MASKS */

/*
 * HcControl (control) register masks
 */
#define OHCI_CTRL_CBSR	(3 << 0)	/* control/bulk service ratio */
#define OHCI_CTRL_PLE	(1 << 2)	/* periodic list enable */
#define OHCI_CTRL_IE	(1 << 3)	/* isochronous enable */
#define OHCI_CTRL_CLE	(1 << 4)	/* control list enable */
#define OHCI_CTRL_BLE	(1 << 5)	/* bulk list enable */
#define OHCI_CTRL_HCFS	(3 << 6)	/* host controller functional state */
#define OHCI_CTRL_IR	(1 << 8)	/* interrupt routing */
#define OHCI_CTRL_RWC	(1 << 9)	/* remote wakeup connected */
#define OHCI_CTRL_RWE	(1 << 10)	/* remote wakeup enable */

/* pre-shifted values for HCFS */
#	define OHCI_USB_RESET	(0 << 6)
#	define OHCI_USB_RESUME	(1 << 6)
#	define OHCI_USB_OPER	(2 << 6)
#	define OHCI_USB_SUSPEND (3 << 6)

/*
 * HcCommandStatus (cmdstatus) register masks
 */
#define OHCI_HCR	(1 << 0)	/* host controller reset */
#define OHCI_CLF	(1 << 1)	/* control list filled */
#define OHCI_BLF	(1 << 2)	/* bulk list filled */
#define OHCI_OCR	(1 << 3)	/* ownership change request */
#define OHCI_SOC	(3 << 16)	/* scheduling overrun count */

/*
 * masks used with interrupt registers:
 * HcInterruptStatus (intrstatus)
 * HcInterruptEnable (intrenable)
 * HcInterruptDisable (intrdisable)
 */
#define OHCI_INTR_SO	(1 << 0)	/* scheduling overrun */
#define OHCI_INTR_WDH	(1 << 1)	/* writeback of done_head */
#define OHCI_INTR_SF	(1 << 2)	/* start frame */
#define OHCI_INTR_RD	(1 << 3)	/* resume detect */
#define OHCI_INTR_UE	(1 << 4)	/* unrecoverable error */
#define OHCI_INTR_FNO	(1 << 5)	/* frame number overflow */
#define OHCI_INTR_RHSC	(1 << 6)	/* root hub status change */
#define OHCI_INTR_OC	(1 << 30)	/* ownership change */
#define OHCI_INTR_MIE	(1 << 31)	/* master interrupt enable */


/* Virtual Root HUB */
struct virt_root_hub
{
	int devnum; /* Address of Root Hub endpoint */
	void *dev;  /* was urb */
	void *int_addr;
	int send;
	int interval;
};

/* USB HUB CONSTANTS (not OHCI-specific; see hub.h) */

/* destination of request */
#define RH_INTERFACE		   0x01
#define RH_ENDPOINT		   0x02
#define RH_OTHER		   0x03

#define RH_CLASS		   0x20
#define RH_VENDOR		   0x40

/* Requests: bRequest << 8 | bmRequestType */
#define RH_GET_STATUS		0x0080
#define RH_CLEAR_FEATURE	0x0100
#define RH_SET_FEATURE		0x0300
#define RH_SET_ADDRESS		0x0500
#define RH_GET_DESCRIPTOR	0x0680
#define RH_SET_DESCRIPTOR	0x0700
#define RH_GET_CONFIGURATION	0x0880
#define RH_SET_CONFIGURATION	0x0900
#define RH_GET_STATE		0x0280
#define RH_GET_INTERFACE	0x0A80
#define RH_SET_INTERFACE	0x0B00
#define RH_SYNC_FRAME		0x0C80
/* Our Vendor Specific Request */
#define RH_SET_EP		0x2000


/* Hub port features */
#define RH_PORT_CONNECTION	   0x00
#define RH_PORT_ENABLE		   0x01
#define RH_PORT_SUSPEND		   0x02
#define RH_PORT_OVER_CURRENT	   0x03
#define RH_PORT_RESET		   0x04
#define RH_PORT_POWER		   0x08
#define RH_PORT_LOW_SPEED	   0x09

#define RH_C_PORT_CONNECTION	   0x10
#define RH_C_PORT_ENABLE	   0x11
#define RH_C_PORT_SUSPEND	   0x12
#define RH_C_PORT_OVER_CURRENT	   0x13
#define RH_C_PORT_RESET		   0x14

/* Hub features */
#define RH_C_HUB_LOCAL_POWER	   0x00
#define RH_C_HUB_OVER_CURRENT	   0x01

#define RH_DEVICE_REMOTE_WAKEUP	   0x00
#define RH_ENDPOINT_STALL	   0x01

#define RH_ACK			   0x01
#define RH_REQ_ERR		   -1
#define RH_NACK			   0x00


/* OHCI ROOT HUB REGISTER MASKS */

/* roothub.portstatus [i] bits */
#define RH_PS_CCS	     0x00000001		/* current connect status */
#define RH_PS_PES	     0x00000002		/* port enable status*/
#define RH_PS_PSS	     0x00000004		/* port suspend status */
#define RH_PS_POCI	     0x00000008		/* port over current indicator */
#define RH_PS_PRS	     0x00000010		/* port reset status */
#define RH_PS_PPS	     0x00000100		/* port power status */
#define RH_PS_LSDA	     0x00000200		/* low speed device attached */
#define RH_PS_CSC	     0x00010000		/* connect status change */
#define RH_PS_PESC	     0x00020000		/* port enable status change */
#define RH_PS_PSSC	     0x00040000		/* port suspend status change */
#define RH_PS_OCIC	     0x00080000		/* over current indicator change */
#define RH_PS_PRSC	     0x00100000		/* port reset status change */

/* roothub.status bits */
#define RH_HS_LPS	     0x00000001		/* local power status */
#define RH_HS_OCI	     0x00000002		/* over current indicator */
#define RH_HS_DRWE	     0x00008000		/* device remote wakeup enable */
#define RH_HS_LPSC	     0x00010000		/* local power status change */
#define RH_HS_OCIC	     0x00020000		/* over current indicator change */
#define RH_HS_CRWE	     0x80000000		/* clear remote wakeup enable */

/* roothub.b masks */
#define RH_B_DR		0x0000ffff		/* device removable flags */
#define RH_B_PPCM	0xffff0000		/* port power control mask */

/* roothub.a masks */
#define RH_A_NDP	(0xff << 0)		/* number of downstream ports */
#define RH_A_PSM	(1 << 8)		/* power switching mode */
#define RH_A_NPS	(1 << 9)		/* no power switching */
#define RH_A_DT		(1 << 10)		/* device type (mbz) */
#define RH_A_OCPM	(1 << 11)		/* over current protection mode */
#define RH_A_NOCP	(1 << 12)		/* no over current protection */
#define RH_A_POTPGT	(0xff << 24)		/* power on to power good time */

/* urb */
#define N_URB_TD 48
typedef struct
{
	ed_t *ed;
	uint16_t length;	/* number of tds associated with this request */
	uint16_t td_cnt;	/* number of tds already serviced */
	struct usb_device *dev;
	int   state;
	uint32_t pipe;
	void *transfer_buffer;
	int transfer_buffer_length;
	int interval;
	int actual_length;
	int finished;
	td_t *td[N_URB_TD];	/* list pointer to all corresponding TDs associated with this request */
} urb_priv_t;
#define URB_DEL 1

#define NUM_EDS 8		/* num of preallocated endpoint descriptors */

struct ohci_device
{
	ed_t	ed[NUM_EDS];
	int ed_cnt;
};

/*
 * This is the full ohci controller description
 *
 * Note how the "proper" USB information is just
 * a subset of what the full implementation needs. (Linus)
 */

typedef struct ohci
{
	/* ------- common part -------- */
	long handle;              /* PCI BIOS */
	const struct pci_device_id *ent;
	int usbnum;
  /* ---- end of common part ---- */
	int big_endian;           /* PCI BIOS */
	int controller;
	struct ohci_hcca *hcca_unaligned;
	struct ohci_hcca *hcca;		/* hcca */
	td_t *td_unaligned;
	struct ohci_device *ohci_dev_unaligned;
	/* this allocates EDs for all possible endpoints */
	struct ohci_device *ohci_dev;

	int irq_enabled;
	int stat_irq;
	int irq;
	int disabled;			/* e.g. got a UE, we're hung */
	int sleeping;
#define OHCI_FLAGS_NEC 0x80000000
	uint32_t flags;		/* for HC bugs */

	uint32_t offset;
	uint32_t dma_offset;
	struct ohci_regs *regs; /* OHCI controller's memory */

	int ohci_int_load[32];	 /* load of the 32 Interrupt Chains (for load balancing)*/
	ed_t *ed_rm_list[2];	 /* lists of all endpoints to be removed */
	ed_t *ed_bulktail;	 /* last endpoint of bulk list */
	ed_t *ed_controltail;	 /* last endpoint of control list */
	int intrstatus;
	uint32_t hc_control;		/* copy of the hc control reg */
	uint32_t ndp;          /* copy NDP from roothub_a */
	struct virt_root_hub rh;

	const char *slot_name;

	/* device which was disconnected */
	struct usb_device *devgone;
} ohci_t;

/* hcd */
/* endpoint */
static int ep_link(ohci_t * ohci, ed_t * ed);
static int ep_unlink(ohci_t * ohci, ed_t * ed);
static ed_t * ep_add_ed(ohci_t * ohci, struct usb_device * usb_dev, uint32_t pipe, int interval, int load);


/* we need more TDs than EDs */
#define NUM_TD 64


static inline void ed_free(struct ed *ed)
{
	ed->usb_dev = NULL;
}


