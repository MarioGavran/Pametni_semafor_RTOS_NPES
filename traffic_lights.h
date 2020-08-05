// Horizontal traffic lights output macros
#define	RED_H		GPIO_ODR_OD10
#define	YELLOW_H	GPIO_ODR_OD6
#define	GREEN_H		GPIO_ODR_OD7


// Horizontal traffic lights input macros
#define	RED_H_i		GPIO_IDR_ID10
#define	YELLOW_H_i	GPIO_IDR_ID6
#define	GREEN_H_i	GPIO_IDR_ID7


// Vertical traffic lights output macros
#define	RED_V		GPIO_ODR_OD0
#define	YELLOW_V	GPIO_ODR_OD1
#define	GREEN_V		GPIO_ODR_OD4


// Vertical traffic lights input macros
#define	RED_V_i		GPIO_IDR_ID0
#define	YELLOW_V_i	GPIO_IDR_ID1
#define	GREEN_V_i	GPIO_IDR_ID4


// Pedestrian button input macros
#define	PEDES_H		GPIO_IDR_ID8
#define	PEDES_V		GPIO_IDR_ID15


// Vehicle count input macros
#define	VEHICLE_H	GPIO_IDR_ID12
#define	VEHICLE_V	GPIO_IDR_ID11


// Function prototypes
extern void blink_test(void);
extern void HORon_VERoff(void);
extern void HORoff_VERon(void);

