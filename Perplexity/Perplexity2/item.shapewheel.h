#ifndef PERPLEXITY_ITEM_SHAPEWHEEL
#define PERPLEXITY_ITEM_SHAPEWHEEL

#include "item.h"
#include "list.h"

extern const char *ShapeWheelNames[];

typedef class ItemShapeWheel : public Item
{
	public:
		typedef enum ShapeEnum {
			Square,
			Circle,
			Triangle,
			Star,
			Hexagon,
			Octagon,
			Plus,
			Minus,
			Infinity,
			COUNT
		} ShapeEnum;

		ItemShapeWheel(int ShapeCount);
		int HandleAction(char **Commands, Room *CurRoom, Person *CurPerson, int OnPerson);
		int HasUse();
		ShapeEnum GetShape();
		List *GetShapeList();
		void RemoveDoor();

	private:
		List *_ShapeList;
		size_t _ShapeCount;
		size_t _CurrentShape;
		void *_Door;

} ItemShapeWheel;

#endif