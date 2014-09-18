// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UMGEditorPrivatePCH.h"

#define LOCTEXT_NAMESPACE "UMG"

const float SnapDistance = 7;

static float DistancePointToLine2D(const FVector2D& LinePointA, const FVector2D& LinePointB, const FVector2D& PointC)
{
	FVector2D AB = LinePointB - LinePointA;
	FVector2D AC = PointC - LinePointA;

	float Distance = FVector2D::CrossProduct(AB, AC) / FVector2D::Distance(LinePointA, LinePointB);
	return FMath::Abs(Distance);
}

FCanvasSlotExtension::FCanvasSlotExtension()
	: bMovingAnchor(false)
{
	ExtensionId = FName(TEXT("CanvasSlot"));
}

bool FCanvasSlotExtension::CanExtendSelection(const TArray< FWidgetReference >& Selection) const
{
	for ( const FWidgetReference& Widget : Selection )
	{
		if ( !Widget.GetTemplate()->Slot || !Widget.GetTemplate()->Slot->IsA(UCanvasPanelSlot::StaticClass()) )
		{
			return false;
		}
	}

	return Selection.Num() == 1;
}

void FCanvasSlotExtension::ExtendSelection(const TArray< FWidgetReference >& Selection, TArray< TSharedRef<FDesignerSurfaceElement> >& SurfaceElements)
{
	SelectionCache = Selection;

	AnchorWidgets.SetNumZeroed(EAnchorWidget::MAX_COUNT);
	AnchorWidgets[EAnchorWidget::Center] = MakeAnchorWidget(EAnchorWidget::Center, 16, 16);

	AnchorWidgets[EAnchorWidget::Left] = MakeAnchorWidget(EAnchorWidget::Left, 32, 16);
	AnchorWidgets[EAnchorWidget::Right] = MakeAnchorWidget(EAnchorWidget::Right, 32, 16);
	AnchorWidgets[EAnchorWidget::Top] = MakeAnchorWidget(EAnchorWidget::Top, 16, 32);
	AnchorWidgets[EAnchorWidget::Bottom] = MakeAnchorWidget(EAnchorWidget::Bottom, 16, 32);

	AnchorWidgets[EAnchorWidget::TopLeft] = MakeAnchorWidget(EAnchorWidget::TopLeft, 24, 24);
	AnchorWidgets[EAnchorWidget::TopRight] = MakeAnchorWidget(EAnchorWidget::TopRight, 24, 24);
	AnchorWidgets[EAnchorWidget::BottomLeft] = MakeAnchorWidget(EAnchorWidget::BottomLeft, 24, 24);
	AnchorWidgets[EAnchorWidget::BottomRight] = MakeAnchorWidget(EAnchorWidget::BottomRight, 24, 24);


	TArray<FVector2D> AnchorPos;
	AnchorPos.SetNumZeroed(EAnchorWidget::MAX_COUNT);
	AnchorPos[EAnchorWidget::Center] = FVector2D(-8, -8);
	
	AnchorPos[EAnchorWidget::Left] = FVector2D(-32, -8);
	AnchorPos[EAnchorWidget::Right] = FVector2D(0, -8);
	AnchorPos[EAnchorWidget::Top] = FVector2D(-8, -32);
	AnchorPos[EAnchorWidget::Bottom] = FVector2D(-8, 0);

	AnchorPos[EAnchorWidget::TopLeft] = FVector2D(-24, -24);
	AnchorPos[EAnchorWidget::TopRight] = FVector2D(0, -24);
	AnchorPos[EAnchorWidget::BottomLeft] = FVector2D(-24, 0);
	AnchorPos[EAnchorWidget::BottomRight] = FVector2D(0, 0);

	for ( int32 AnchorIndex = (int32)EAnchorWidget::MAX_COUNT - 1; AnchorIndex >= 0; AnchorIndex-- )
	{
		if ( !AnchorWidgets[AnchorIndex].IsValid() )
		{
			continue;
		}

		AnchorWidgets[AnchorIndex]->SlatePrepass();
		TAttribute<FVector2D> AnchorAlignment = TAttribute<FVector2D>::Create(TAttribute<FVector2D>::FGetter::CreateSP(SharedThis(this), &FCanvasSlotExtension::GetAnchorAlignment, (EAnchorWidget::Type)AnchorIndex));
		SurfaceElements.Add(MakeShareable(new FDesignerSurfaceElement(AnchorWidgets[AnchorIndex].ToSharedRef(), EExtensionLayoutLocation::Absolute, AnchorPos[AnchorIndex], AnchorAlignment)));
	}
}

TSharedRef<SWidget> FCanvasSlotExtension::MakeAnchorWidget(EAnchorWidget::Type AnchorType, float Width, float Height)
{
	return SNew(SBorder)
		.BorderImage(FEditorStyle::Get().GetBrush("NoBrush"))
		.OnMouseButtonDown(this, &FCanvasSlotExtension::HandleAnchorBeginDrag, AnchorType)
		.OnMouseButtonUp(this, &FCanvasSlotExtension::HandleAnchorEndDrag, AnchorType)
		.OnMouseMove(this, &FCanvasSlotExtension::HandleAnchorDragging, AnchorType)
		.Visibility(this, &FCanvasSlotExtension::GetAnchorVisibility, AnchorType)
		.Padding(FMargin(0))
		[
			SNew(SBox)
			.WidthOverride(Width)
			.HeightOverride(Height)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SImage)
				.Image(this, &FCanvasSlotExtension::GetAnchorBrush, AnchorType)
			]
		];
}

const FSlateBrush* FCanvasSlotExtension::GetAnchorBrush(EAnchorWidget::Type AnchorType) const
{
	switch ( AnchorType )
	{
	case EAnchorWidget::Center:
		return AnchorWidgets[EAnchorWidget::Center]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Center.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Center");
	case EAnchorWidget::Left:
		return AnchorWidgets[EAnchorWidget::Left]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Left.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Left");
	case EAnchorWidget::Right:
		return AnchorWidgets[EAnchorWidget::Right]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Right.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Right");
	case EAnchorWidget::Top:
		return AnchorWidgets[EAnchorWidget::Top]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Top.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Top");
	case EAnchorWidget::Bottom:
		return AnchorWidgets[EAnchorWidget::Bottom]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Bottom.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.Bottom");
	case EAnchorWidget::TopLeft:
		return AnchorWidgets[EAnchorWidget::TopLeft]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.TopLeft.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.TopLeft");
	case EAnchorWidget::BottomRight:
		return AnchorWidgets[EAnchorWidget::BottomRight]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.BottomRight.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.BottomRight");
	case EAnchorWidget::TopRight:
		return AnchorWidgets[EAnchorWidget::TopRight]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.TopRight.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.TopRight");
	case EAnchorWidget::BottomLeft:
		return AnchorWidgets[EAnchorWidget::BottomLeft]->IsHovered() ? FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.BottomLeft.Hovered") : FEditorStyle::Get().GetBrush("UMGEditor.AnchorGizmo.BottomLeft");
	}

	return FCoreStyle::Get().GetBrush("Selection");
}

EVisibility FCanvasSlotExtension::GetAnchorVisibility(EAnchorWidget::Type AnchorType) const
{
	for ( const FWidgetReference& Selection : SelectionCache )
	{
		UWidget* PreviewWidget = Selection.GetPreview();
		if ( PreviewWidget && PreviewWidget->Slot )
		{
			if ( UCanvasPanelSlot* PreviewCanvasSlot = Cast<UCanvasPanelSlot>(PreviewWidget->Slot) )
			{
				switch ( AnchorType )
				{
				case EAnchorWidget::Center:
					return PreviewCanvasSlot->LayoutData.Anchors.Minimum == PreviewCanvasSlot->LayoutData.Anchors.Maximum ? EVisibility::Visible : EVisibility::Collapsed;
				case EAnchorWidget::Left:
				case EAnchorWidget::Right:
					return PreviewCanvasSlot->LayoutData.Anchors.Minimum.Y == PreviewCanvasSlot->LayoutData.Anchors.Maximum.Y ? EVisibility::Visible : EVisibility::Collapsed;
				case EAnchorWidget::Top:
				case EAnchorWidget::Bottom:
					return PreviewCanvasSlot->LayoutData.Anchors.Minimum.X == PreviewCanvasSlot->LayoutData.Anchors.Maximum.X ? EVisibility::Visible : EVisibility::Collapsed;
				}

				return EVisibility::Visible;
			}
		}
	}

	return EVisibility::Collapsed;
}

FVector2D FCanvasSlotExtension::GetAnchorAlignment(EAnchorWidget::Type AnchorType) const
{
	for ( const FWidgetReference& Selection : SelectionCache )
	{
		UWidget* PreviewWidget = Selection.GetPreview();
		if ( PreviewWidget && PreviewWidget->Slot )
		{
			if ( UCanvasPanelSlot* PreviewCanvasSlot = Cast<UCanvasPanelSlot>(PreviewWidget->Slot) )
			{
				FVector2D Minimum = PreviewCanvasSlot->LayoutData.Anchors.Minimum;
				FVector2D Maximum = PreviewCanvasSlot->LayoutData.Anchors.Maximum;

				switch ( AnchorType )
				{
				case EAnchorWidget::Center:
				case EAnchorWidget::Left:
				case EAnchorWidget::Top:
				case EAnchorWidget::TopLeft:
					return Minimum;
				case EAnchorWidget::Right:
				case EAnchorWidget::Bottom:
				case EAnchorWidget::BottomRight:
					return Maximum;
				case EAnchorWidget::TopRight:
					return  FVector2D(Maximum.X, Minimum.Y);
				case EAnchorWidget::BottomLeft:
					return  FVector2D(Minimum.X, Maximum.Y);
				}
			}
		}
	}

	return FVector2D(0, 0);
}

bool FCanvasSlotExtension::GetCollisionSegmentsForSlot(UCanvasPanel* Canvas, int32 SlotIndex, TArray<FVector2D>& Segments)
{
	FGeometry ArrangedGeometry;
	if ( Canvas->GetGeometryForSlot(SlotIndex, ArrangedGeometry) )
	{
		GetCollisionSegmentsFromGeometry(ArrangedGeometry, Segments);
		return true;
	}
	return false;
}

bool FCanvasSlotExtension::GetCollisionSegmentsForSlot(UCanvasPanel* Canvas, UCanvasPanelSlot* Slot, TArray<FVector2D>& Segments)
{
	FGeometry ArrangedGeometry;
	if ( Canvas->GetGeometryForSlot(Slot, ArrangedGeometry) )
	{
		GetCollisionSegmentsFromGeometry(ArrangedGeometry, Segments);
		return true;
	}
	return false;
}

void FCanvasSlotExtension::GetCollisionSegmentsFromGeometry(FGeometry ArrangedGeometry, TArray<FVector2D>& Segments)
{
	Segments.SetNumUninitialized(8);

	// Left Side
	Segments[0] = ArrangedGeometry.Position;
	Segments[1] = ArrangedGeometry.Position + FVector2D(0, ArrangedGeometry.Size.Y);

	// Top Side
	Segments[2] = ArrangedGeometry.Position;
	Segments[3] = ArrangedGeometry.Position + FVector2D(ArrangedGeometry.Size.X, 0);

	// Right Side
	Segments[4] = ArrangedGeometry.Position + FVector2D(ArrangedGeometry.Size.X, 0);
	Segments[5] = ArrangedGeometry.Position + ArrangedGeometry.Size;

	// Bottom Side
	Segments[6] = ArrangedGeometry.Position + FVector2D(0, ArrangedGeometry.Size.Y);
	Segments[7] = ArrangedGeometry.Position + ArrangedGeometry.Size;
}

void FCanvasSlotExtension::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{

}

void FCanvasSlotExtension::Paint(const TSet< FWidgetReference >& Selection, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	PaintCollisionLines(Selection, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
}

FReply FCanvasSlotExtension::HandleAnchorBeginDrag(const FGeometry& Geometry, const FPointerEvent& Event, EAnchorWidget::Type AnchorType)
{
	BeginTransaction(LOCTEXT("MoveAnchor", "Move Anchor"));

	bMovingAnchor = true;
	MouseDownPosition = Event.GetScreenSpacePosition();

	UCanvasPanelSlot* PreviewCanvasSlot = Cast<UCanvasPanelSlot>(SelectionCache[0].GetPreview()->Slot);
	BeginAnchors = PreviewCanvasSlot->LayoutData.Anchors;

	return FReply::Handled().CaptureMouse(AnchorWidgets[(int32)AnchorType].ToSharedRef());
}

FReply FCanvasSlotExtension::HandleAnchorEndDrag(const FGeometry& Geometry, const FPointerEvent& Event, EAnchorWidget::Type AnchorType)
{
	EndTransaction();

	bMovingAnchor = false;
	return FReply::Handled().ReleaseMouseCapture();
}

void FCanvasSlotExtension::PoximitySnapValue(float SnapFrequency, float SnapProximity, float& Value)
{
	float MajorAnchorDiv = Value / SnapFrequency;
	float SubAnchorLinePos = MajorAnchorDiv - FMath::Round(MajorAnchorDiv);

	if ( FMath::Abs(SubAnchorLinePos) <= SnapProximity )
	{
		Value = FMath::Round(MajorAnchorDiv) * SnapFrequency;
	}
}

FReply FCanvasSlotExtension::HandleAnchorDragging(const FGeometry& Geometry, const FPointerEvent& Event, EAnchorWidget::Type AnchorType)
{
	if ( bMovingAnchor && !Event.GetCursorDelta().IsZero() )
	{
		float InverseSize = 1.0f / Designer->GetPreviewScale();

		for ( FWidgetReference& Selection : SelectionCache )
		{
			UWidget* PreviewWidget = Selection.GetPreview();
			if ( UCanvasPanel* Canvas = Cast<UCanvasPanel>(PreviewWidget->GetParent()) )
			{
				UCanvasPanelSlot* PreviewCanvasSlot = Cast<UCanvasPanelSlot>(PreviewWidget->Slot);

				FGeometry Geometry;
				if ( Canvas->GetGeometryForSlot(PreviewCanvasSlot, Geometry) )
				{
					FGeometry CanvasGeometry = Canvas->GetCanvasWidget()->GetCachedGeometry();
					FVector2D StartLocalPosition = CanvasGeometry.AbsoluteToLocal(MouseDownPosition);
					FVector2D NewLocalPosition = CanvasGeometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
					FVector2D LocalPositionDelta = NewLocalPosition - StartLocalPosition;

					FVector2D AnchorDelta = LocalPositionDelta / CanvasGeometry.Size;

					const FAnchorData OldLayoutData = PreviewCanvasSlot->LayoutData;
					FAnchorData LayoutData = OldLayoutData;
					
					switch ( AnchorType )
					{
					case EAnchorWidget::Center:
						LayoutData.Anchors.Maximum = BeginAnchors.Maximum + AnchorDelta;
						LayoutData.Anchors.Minimum = BeginAnchors.Minimum + AnchorDelta;
						LayoutData.Anchors.Minimum.X = FMath::Clamp(LayoutData.Anchors.Minimum.X, 0.0f, 1.0f);
						LayoutData.Anchors.Maximum.X = FMath::Clamp(LayoutData.Anchors.Maximum.X, 0.0f, 1.0f);
						LayoutData.Anchors.Minimum.Y = FMath::Clamp(LayoutData.Anchors.Minimum.Y, 0.0f, 1.0f);
						LayoutData.Anchors.Maximum.Y = FMath::Clamp(LayoutData.Anchors.Maximum.Y, 0.0f, 1.0f);
						break;
					}

					switch ( AnchorType )
					{
					case EAnchorWidget::Left:
					case EAnchorWidget::TopLeft:
					case EAnchorWidget::BottomLeft:
						LayoutData.Anchors.Minimum.X = BeginAnchors.Minimum.X + AnchorDelta.X;
						LayoutData.Anchors.Minimum.X = FMath::Clamp(LayoutData.Anchors.Minimum.X, 0.0f, LayoutData.Anchors.Maximum.X);
						break;
					}

					switch ( AnchorType )
					{
					case EAnchorWidget::Right:
					case EAnchorWidget::TopRight:
					case EAnchorWidget::BottomRight:
						LayoutData.Anchors.Maximum.X = BeginAnchors.Maximum.X + AnchorDelta.X;
						LayoutData.Anchors.Maximum.X = FMath::Clamp(LayoutData.Anchors.Maximum.X, LayoutData.Anchors.Minimum.X, 1.0f);
						break;
					}

					switch ( AnchorType )
					{
					case EAnchorWidget::Top:
					case EAnchorWidget::TopLeft:
					case EAnchorWidget::TopRight:
						LayoutData.Anchors.Minimum.Y = BeginAnchors.Minimum.Y + AnchorDelta.Y;
						LayoutData.Anchors.Minimum.Y = FMath::Clamp(LayoutData.Anchors.Minimum.Y, 0.0f, LayoutData.Anchors.Maximum.Y);
						break;
					}

					switch ( AnchorType )
					{
					case EAnchorWidget::Bottom:
					case EAnchorWidget::BottomLeft:
					case EAnchorWidget::BottomRight:
						LayoutData.Anchors.Maximum.Y = BeginAnchors.Maximum.Y + AnchorDelta.Y;
						LayoutData.Anchors.Maximum.Y = FMath::Clamp(LayoutData.Anchors.Maximum.Y, LayoutData.Anchors.Minimum.Y, 1.0f);
						break;
					}

					// Major percentage snapping
					{
						const float MajorAnchorLine = 0.1f;
						const float MajorAnchorLineSnapDistance = 0.1f;

						if ( LayoutData.Anchors.Minimum.X != OldLayoutData.Anchors.Minimum.X )
						{
							PoximitySnapValue(MajorAnchorLine, MajorAnchorLineSnapDistance, LayoutData.Anchors.Minimum.X);
						}

						if ( LayoutData.Anchors.Minimum.Y != OldLayoutData.Anchors.Minimum.Y )
						{
							PoximitySnapValue(MajorAnchorLine, MajorAnchorLineSnapDistance, LayoutData.Anchors.Minimum.Y);
						}

						if ( LayoutData.Anchors.Maximum.X != OldLayoutData.Anchors.Maximum.X )
						{
							PoximitySnapValue(MajorAnchorLine, MajorAnchorLineSnapDistance, LayoutData.Anchors.Maximum.X);
						}

						if ( LayoutData.Anchors.Maximum.Y != OldLayoutData.Anchors.Maximum.Y )
						{
							PoximitySnapValue(MajorAnchorLine, MajorAnchorLineSnapDistance, LayoutData.Anchors.Maximum.Y);
						}
					}

					// Rebase the layout and restore the old value after calculating the new final layout
					// result.
					{
						PreviewCanvasSlot->SaveBaseLayout();
						PreviewCanvasSlot->LayoutData = LayoutData;
						PreviewCanvasSlot->RebaseLayout();

						LayoutData = PreviewCanvasSlot->LayoutData;
						PreviewCanvasSlot->LayoutData = OldLayoutData;
					}

					// If control is pressed reset all positional offset information
					if ( FSlateApplication::Get().GetModifierKeys().IsControlDown() )
					{
						FMargin NewOffsets = FMargin(0, 0, LayoutData.Anchors.IsStretchedHorizontal() ? 0 : LayoutData.Offsets.Right, LayoutData.Anchors.IsStretchedVertical() ? 0 : LayoutData.Offsets.Bottom);
						LayoutData.Offsets = NewOffsets;
					}

					UWidget* TemplateWidget = Selection.GetTemplate();
					UCanvasPanelSlot* TemplateCanvasSlot = CastChecked<UCanvasPanelSlot>(TemplateWidget->Slot);

					static const FName LayoutDataName(TEXT("LayoutData"));

					FObjectEditorUtils::SetPropertyValue<UCanvasPanelSlot, FAnchorData>(PreviewCanvasSlot, LayoutDataName, LayoutData);
					FObjectEditorUtils::SetPropertyValue<UCanvasPanelSlot, FAnchorData>(TemplateCanvasSlot, LayoutDataName, LayoutData);
				}
			};

			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

void FCanvasSlotExtension::PaintCollisionLines(const TSet< FWidgetReference >& Selection, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	for ( const FWidgetReference& WidgetRef : Selection )
	{
		if ( !WidgetRef.IsValid() )
		{
			continue;
		}

		UWidget* Widget = WidgetRef.GetPreview();

		if ( UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot) )
		{
			if ( UCanvasPanel* Canvas = CastChecked<UCanvasPanel>(CanvasSlot->Parent) )
			{
				//TODO UMG Only show guide lines when near them and dragging
				if ( false )
				{
					FGeometry MyArrangedGeometry;
					if ( !Canvas->GetGeometryForSlot(CanvasSlot, MyArrangedGeometry) )
					{
						continue;
					}

					TArray<FVector2D> LinePoints;
					LinePoints.AddUninitialized(2);

					// Get the collision segments that we could potentially be docking against.
					TArray<FVector2D> MySegments;
					if ( GetCollisionSegmentsForSlot(Canvas, CanvasSlot, MySegments) )
					{
						for ( int32 MySegmentIndex = 0; MySegmentIndex < MySegments.Num(); MySegmentIndex += 2 )
						{
							FVector2D MySegmentBase = MySegments[MySegmentIndex];

							for ( int32 SlotIndex = 0; SlotIndex < Canvas->GetChildrenCount(); SlotIndex++ )
							{
								// Ignore the slot being dragged.
								if ( Canvas->Slots[SlotIndex] == CanvasSlot )
								{
									continue;
								}

								// Get the collision segments that we could potentially be docking against.
								TArray<FVector2D> Segments;
								if ( GetCollisionSegmentsForSlot(Canvas, SlotIndex, Segments) )
								{
									for ( int32 SegmentBase = 0; SegmentBase < Segments.Num(); SegmentBase += 2 )
									{
										FVector2D PointA = Segments[SegmentBase];
										FVector2D PointB = Segments[SegmentBase + 1];

										FVector2D CollisionPoint = MySegmentBase;

										//TODO Collide against all sides of the arranged geometry.
										float Distance = DistancePointToLine2D(PointA, PointB, CollisionPoint);
										if ( Distance <= SnapDistance )
										{
											FVector2D FarthestPoint = FVector2D::Distance(PointA, CollisionPoint) > FVector2D::Distance(PointB, CollisionPoint) ? PointA : PointB;
											FVector2D NearestPoint = FVector2D::Distance(PointA, CollisionPoint) > FVector2D::Distance(PointB, CollisionPoint) ? PointB : PointA;

											LinePoints[0] = FarthestPoint;
											LinePoints[1] = FarthestPoint + ( NearestPoint - FarthestPoint ) * 100000;

											LinePoints[0].X = FMath::Clamp(LinePoints[0].X, 0.0f, MyClippingRect.Right - MyClippingRect.Left);
											LinePoints[0].Y = FMath::Clamp(LinePoints[0].Y, 0.0f, MyClippingRect.Bottom - MyClippingRect.Top);

											LinePoints[1].X = FMath::Clamp(LinePoints[1].X, 0.0f, MyClippingRect.Right - MyClippingRect.Left);
											LinePoints[1].Y = FMath::Clamp(LinePoints[1].Y, 0.0f, MyClippingRect.Bottom - MyClippingRect.Top);

											FLinearColor Color(0.5f, 0.75, 1);
											const bool bAntialias = true;

											FSlateDrawElement::MakeLines(
												OutDrawElements,
												LayerId,
												AllottedGeometry.ToPaintGeometry(),
												LinePoints,
												MyClippingRect,
												ESlateDrawEffect::None,
												Color,
												bAntialias);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
